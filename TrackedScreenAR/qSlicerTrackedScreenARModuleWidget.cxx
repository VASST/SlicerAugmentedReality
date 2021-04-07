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
#include <vtkMRMLVideoCameraNode.h>

// ITK includes
#include <vnl_math.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkTexture.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerTrackedScreenARModuleWidgetPrivate: public Ui_qSlicerTrackedScreenARModuleWidget
{
public:
  vtkMRMLLinearTransformNode* cameraTransformNode = nullptr;
  vtkMRMLVideoCameraNode* cameraParametersNode = nullptr;
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
  if (node != nullptr && vtkMRMLVideoCameraNode::SafeDownCast(node) != nullptr && d->videoSourceNode != nullptr)
  {
    vtkMRMLVideoCameraNode* videoCameraNode = vtkMRMLVideoCameraNode::SafeDownCast(node);
    d->cameraParametersNode = videoCameraNode;

    vtkCamera* camera = qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->cameraNode()->GetCamera();

    double xPixels = d->videoSourceNode->GetImageData()->GetDimensions()[0];
    double yPixels = d->videoSourceNode->GetImageData()->GetDimensions()[1];

    // Courtesy of https://gist.github.com/decrispell/fc4b69f6bedf07a3425b
    // convert the principal point to window center (normalized coordinate system) and set it
    double wcx = -2 * (videoCameraNode->GetIntrinsicMatrix()->GetElement(0, 2) - double(xPixels) / 2) / xPixels;
    double wcy = 2 * (videoCameraNode->GetIntrinsicMatrix()->GetElement(1, 2) - double(yPixels) / 2) / yPixels;
    camera->SetWindowCenter(wcx, wcy);

    // convert the focal length to view angle and set it
    double view_angle = vnl_math::deg_per_rad * (2.0 * std::atan2(yPixels / 2.0, videoCameraNode->GetIntrinsicMatrix()->GetElement(1, 1)));
    camera->SetViewAngle(view_angle);
  }
  else
  {
    d->cameraParametersNode = nullptr;
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
}
