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

// Slicer includes
#include <qSlicerAbstractModuleRepresentation.h>
#include <qSlicerApplication.h>
#include <qSlicerModuleManager.h>

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>

// SlicerQt includes
#include "qSlicerVideoPassthroughModuleWidget.h"
#include "ui_qSlicerVideoPassthroughModuleWidget.h"

// Local includes
#include "vtkSlicerVideoPassthroughLogic.h"

// SlicerVirtualReality includes
#include <qMRMLVirtualRealityView.h>
#include <qSlicerVirtualRealityModule.h>
#include <qSlicerVirtualRealityModuleWidget.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkTexture.h>

// VTK OpenVR includes
#include <vtkOpenVRRenderer.h>

// OS includes
#include <thread>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_AugmentedReality
class qSlicerVideoPassthroughModuleWidgetPrivate: public Ui_qSlicerVideoPassthroughModuleWidget
{
public:
  qSlicerVideoPassthroughModuleWidgetPrivate();

  qMRMLVirtualRealityView* VRView;

  vtkMRMLScalarVolumeNode* LeftEyeNode = nullptr;
  vtkMRMLScalarVolumeNode* RightEyeNode = nullptr;

  vtkTexture* LeftEyeTexture = vtkTexture::New();
  vtkTexture* RightEyeTexture = vtkTexture::New();

public:
  ~qSlicerVideoPassthroughModuleWidgetPrivate()
  {
    this->LeftEyeTexture->Delete();
    this->RightEyeTexture->Delete();
  }
};

//-----------------------------------------------------------------------------
// qSlicerVideoPassthroughModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVideoPassthroughModuleWidgetPrivate::qSlicerVideoPassthroughModuleWidgetPrivate()
{

}

//-----------------------------------------------------------------------------
// qSlicerVideoPassthroughModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVideoPassthroughModuleWidget::qSlicerVideoPassthroughModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVideoPassthroughModuleWidgetPrivate)
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  qSlicerVirtualRealityModule* module = dynamic_cast<qSlicerVirtualRealityModule*>(qSlicerApplication::application()->moduleManager()->module("VirtualReality"));
  assert(module); // dependency system should have assured us this exists

  // Workaround: find view via findChildren approach as module function is not exported
  qSlicerVirtualRealityModuleWidget* widget = dynamic_cast<qSlicerVirtualRealityModuleWidget*>(module->widgetRepresentation());
  assert(widget); // ensure that widget has been created, lazy creation should guarantee this existence

  d->VRView = module->viewWidget();
}

//-----------------------------------------------------------------------------
qSlicerVideoPassthroughModuleWidget::~qSlicerVideoPassthroughModuleWidget()
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  QWidget::disconnect(d->comboBox_leftEye, static_cast<void (qMRMLNodeComboBox::*)(vtkMRMLNode*)>(&qMRMLNodeComboBox::currentNodeChanged), this, &qSlicerVideoPassthroughModuleWidget::onLeftEyeNodeChanged);
  QWidget::disconnect(d->comboBox_rightEye, static_cast<void (qMRMLNodeComboBox::*)(vtkMRMLNode*)>(&qMRMLNodeComboBox::currentNodeChanged), this, &qSlicerVideoPassthroughModuleWidget::onRightEyeNodeChanged);
}

//----------------------------------------------------------------------------
void qSlicerVideoPassthroughModuleWidget::onLeftEyeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  vtkMRMLScalarVolumeNode* scalarNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  d->LeftEyeNode = scalarNode;
  if (node != nullptr)
  {
    d->LeftEyeTexture->SetInputDataObject(scalarNode->GetImageData());
  }

  eyeChanged();
}

//----------------------------------------------------------------------------
void qSlicerVideoPassthroughModuleWidget::onRightEyeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  vtkMRMLScalarVolumeNode* scalarNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  d->RightEyeNode = scalarNode;
  if (node != nullptr)
  {
    d->RightEyeTexture->SetInputDataObject(scalarNode->GetImageData());
  }

  eyeChanged();
}

//----------------------------------------------------------------------------
void qSlicerVideoPassthroughModuleWidget::eyeChanged()
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  if (d->LeftEyeNode != nullptr && d->RightEyeNode != nullptr)
  {
    d->VRView->renderer()->SetTexturedBackground(true);
    d->VRView->renderer()->SetLeftBackgroundTexture(d->LeftEyeTexture);
    d->VRView->renderer()->SetRightBackgroundTexture(d->RightEyeTexture);
  }
  else
  {
    d->VRView->renderer()->SetTexturedBackground(false);
    d->VRView->renderer()->SetLeftBackgroundTexture(nullptr);
    d->VRView->renderer()->SetRightBackgroundTexture(nullptr);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVideoPassthroughModuleWidget::setup()
{
  Q_D(qSlicerVideoPassthroughModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  QWidget::connect(d->comboBox_leftEye, static_cast<void (qMRMLNodeComboBox::*)(vtkMRMLNode*)>(&qMRMLNodeComboBox::currentNodeChanged), this, &qSlicerVideoPassthroughModuleWidget::onLeftEyeNodeChanged);
  QWidget::connect(d->comboBox_rightEye, static_cast<void (qMRMLNodeComboBox::*)(vtkMRMLNode*)>(&qMRMLNodeComboBox::currentNodeChanged), this, &qSlicerVideoPassthroughModuleWidget::onRightEyeNodeChanged);
}
