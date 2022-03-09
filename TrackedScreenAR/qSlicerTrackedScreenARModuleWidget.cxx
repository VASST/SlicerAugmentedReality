/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>

// Local includes
#include "qSlicerTrackedScreenARModuleWidget.h"
#include "ui_qSlicerTrackedScreenARModuleWidget.h"

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>

// MRML includes
#include <qMRMLThreeDView.h>
#include <qMRMLThreeDWidget.h>
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVectorVolumeNode.h>

// Video cameras include
#include <vtkMRMLPinholeCameraNode.h>

// ITK includes
#include <vnl_math.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkTexture.h>

// VNL includes
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/algo/vnl_matrix_inverse.h>

namespace
{
  //----------------------------------------------------------------------------
  void ConvertVtkMatrixToVnlMatrix(const vtkMatrix4x4* inVtkMatrix, vnl_matrix_fixed<double, 4, 4>& outVnlMatrix)
  {

    for (int row = 0; row < 4; row++)
    {
      for (int column = 0; column < 4; column++)
      {
        outVnlMatrix.put(row, column, inVtkMatrix->GetElement(row, column));
      }
    }
  }

  //----------------------------------------------------------------------------
  void ConvertVnlMatrixToVtkMatrix(const vnl_matrix_fixed<double, 4, 4>& inVnlMatrix, vtkMatrix4x4* outVtkMatrix)
  {
    outVtkMatrix->Identity();

    for (int row = 0; row < 3; row++)
    {
      for (int column = 0; column < 4; column++)
      {
        outVtkMatrix->SetElement(row, column, inVnlMatrix.get(row, column));
      }
    }
  }
}

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerTrackedScreenARModuleWidgetPrivate: public Ui_qSlicerTrackedScreenARModuleWidget
{
public:
  vtkMRMLLinearTransformNode* cameraTransformNode = nullptr;
  vtkMRMLPinholeCameraNode* cameraParametersNode = nullptr;
  vtkMRMLVolumeNode* videoSourceNode = nullptr;

  vtkTexture* BackgroundTexture = nullptr;

  unsigned long ImageObserverTag = 0;

public:
  qSlicerTrackedScreenARModuleWidgetPrivate();
  ~qSlicerTrackedScreenARModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTrackedScreenARModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerTrackedScreenARModuleWidgetPrivate::qSlicerTrackedScreenARModuleWidgetPrivate()
{
  this->BackgroundTexture = vtkTexture::New();
}

//-----------------------------------------------------------------------------
qSlicerTrackedScreenARModuleWidgetPrivate::~qSlicerTrackedScreenARModuleWidgetPrivate()
{
  if (this->BackgroundTexture != nullptr)
  {
    this->BackgroundTexture->Delete();
    this->BackgroundTexture = nullptr;
  }
}

//-----------------------------------------------------------------------------
// qSlicerTrackedScreenARModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTrackedScreenARModuleWidget::qSlicerTrackedScreenARModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTrackedScreenARModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTrackedScreenARModuleWidget::~qSlicerTrackedScreenARModuleWidget()
{
}

//----------------------------------------------------------------------------
void qSlicerTrackedScreenARModuleWidget::onCameraTransformNodeChanged(const QString& nodeId)
{
  Q_D(qSlicerTrackedScreenARModuleWidget);

  vtkMRMLNode* node = this->mrmlScene()->GetNodeByID(nodeId.toStdString());
  if (node == nullptr)
  {
    qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->cameraNode()->SetAndObserveTransformNodeID(nodeId.toStdString().c_str());
    return;
  }

  d->cameraTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);

  // parent camera by transform
  vtkMRMLCameraNode* camera = qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->cameraNode();
  camera->SetAndObserveTransformNodeID(node->GetID());

  this->onResetViewClicked();
}

//----------------------------------------------------------------------------
void qSlicerTrackedScreenARModuleWidget::onVideoSourceNodeChanged(const QString& nodeId)
{
  Q_D(qSlicerTrackedScreenARModuleWidget);

  vtkMRMLNode* node = this->mrmlScene()->GetNodeByID(nodeId.toStdString());

  if (d->videoSourceNode != nullptr && d->videoSourceNode != node)
  {
    d->videoSourceNode->RemoveObserver(d->ImageObserverTag);
    d->ImageObserverTag = 0;
    d->videoSourceNode = nullptr;
  }

  if (node == nullptr || vtkMRMLVolumeNode::SafeDownCast(node)->GetImageData() == nullptr)
  {
    qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->renderWindow()->GetRenderers()->GetFirstRenderer()->SetTexturedBackground(false);
    d->BackgroundTexture->SetInputDataObject(nullptr);
  }
  else
  {
    d->videoSourceNode = vtkMRMLVolumeNode::SafeDownCast(node);

    qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->renderWindow()->GetRenderers()->GetFirstRenderer()->SetTexturedBackground(true);
    d->BackgroundTexture->SetInputDataObject(d->videoSourceNode->GetImageData());
    qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->renderWindow()->GetRenderers()->GetFirstRenderer()->SetLeftBackgroundTexture(d->BackgroundTexture);

    d->ImageObserverTag = d->videoSourceNode->GetImageData()->AddObserver(vtkCommand::ModifiedEvent, this, &qSlicerTrackedScreenARModuleWidget::onImageDataModified);

    // Finally, trigger any camera parameter setting
    if (d->cameraParametersNode != nullptr)
    {
      this->onVideoSourceParametersNodeChanged(QString(d->cameraParametersNode->GetID()));
    }
  }
}

//----------------------------------------------------------------------------
void qSlicerTrackedScreenARModuleWidget::onVideoSourceParametersNodeChanged(const QString& nodeId)
{
  Q_D(qSlicerTrackedScreenARModuleWidget);

  // make VTK camera parameters match new camera intrinsics
  vtkMRMLNode* node = this->mrmlScene()->GetNodeByID(nodeId.toStdString());
  if (node != nullptr && vtkMRMLPinholeCameraNode::SafeDownCast(node) != nullptr && d->videoSourceNode != nullptr)
  {
    vtkMRMLPinholeCameraNode* videoCameraNode = vtkMRMLPinholeCameraNode::SafeDownCast(node);
    d->cameraParametersNode = videoCameraNode;

    vtkCamera* camera = qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->cameraNode()->GetCamera();

    double imageWidth = d->videoSourceNode->GetImageData()->GetDimensions()[0];
    double imageHeight = d->videoSourceNode->GetImageData()->GetDimensions()[1];
    double windowWidth = qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->renderWindow()->GetSize()[0];
    double windowHeight = qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->renderWindow()->GetSize()[1];

    double focalLengthY = videoCameraNode->GetIntrinsicMatrix()->GetElement(1, 1);
    if (windowHeight != imageHeight)
    {
      double factor = static_cast<double>(windowHeight) / static_cast<double>(imageHeight);
      focalLengthY = focalLengthY * factor;
    }

    camera->SetViewAngle(2 * atan((windowHeight / 2) / focalLengthY) * 180 / vnl_math::pi);

    // Calculate window center
    double px = 0;
    double width = 0;

    double py = 0;
    double height = 0;

    if (imageWidth != windowWidth || imageHeight != windowHeight)
    {
      double factor = static_cast<double>(windowHeight) / static_cast<double>(imageHeight);

      px = factor * videoCameraNode->GetIntrinsicMatrix()->GetElement(0, 2);
      width = windowWidth;
      int expectedWindowSize = vtkMath::Round(factor * static_cast<double>(imageWidth));
      if (expectedWindowSize != windowWidth)
      {
        int diffX = (windowWidth - expectedWindowSize) / 2;
        px = px + diffX;
      }

      py = factor * videoCameraNode->GetIntrinsicMatrix()->GetElement(1, 2);
      height = windowHeight;
    }
    else
    {
      px = videoCameraNode->GetIntrinsicMatrix()->GetElement(0, 2);
      width = imageWidth;

      py = videoCameraNode->GetIntrinsicMatrix()->GetElement(1, 2);
      height = imageHeight;
    }

    double cx = width - px;
    double cy = py;

    camera->SetWindowCenter(cx / ((width - 1) / 2) - 1, cy / ((height - 1) / 2) - 1);
  }
  else
  {
    d->cameraParametersNode = nullptr;
  }
}

//----------------------------------------------------------------------------
void qSlicerTrackedScreenARModuleWidget::onResetViewClicked()
{
  Q_D(qSlicerTrackedScreenARModuleWidget);

  vtkMRMLCameraNode* camNode = qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->cameraNode();
  vtkMRMLLinearTransformNode* trNode = vtkMRMLLinearTransformNode::SafeDownCast(d->comboBox_CameraTransform->currentNode());

  if (trNode)
  {
    vtkNew<vtkMatrix4x4> extrinsicTransform;
    trNode->GetMatrixTransformToWorld(extrinsicTransform);
    camNode->GetAppliedTransform()->DeepCopy(extrinsicTransform);
  }
}

//----------------------------------------------------------------------------
void qSlicerTrackedScreenARModuleWidget::onImageDataModified()
{
  qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->scheduleRender();
}

//-----------------------------------------------------------------------------
void qSlicerTrackedScreenARModuleWidget::setup()
{
  Q_D(qSlicerTrackedScreenARModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect(d->comboBox_VideoSource, &qMRMLNodeComboBox::currentNodeIDChanged, this, &qSlicerTrackedScreenARModuleWidget::onVideoSourceNodeChanged);
  connect(d->comboBox_VideoCameraParameters, &qMRMLNodeComboBox::currentNodeIDChanged, this, &qSlicerTrackedScreenARModuleWidget::onVideoSourceParametersNodeChanged);
  connect(d->comboBox_CameraTransform, &qMRMLNodeComboBox::currentNodeIDChanged, this, &qSlicerTrackedScreenARModuleWidget::onCameraTransformNodeChanged);
  connect(d->pushButton_ResetView, &QPushButton::clicked, this, &qSlicerTrackedScreenARModuleWidget::onResetViewClicked);
}
