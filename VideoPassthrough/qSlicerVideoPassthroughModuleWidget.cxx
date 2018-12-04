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
  : Superclass( _parent )
  , d_ptr( new qSlicerVideoPassthroughModuleWidgetPrivate )
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  qSlicerVirtualRealityModule* module = dynamic_cast<qSlicerVirtualRealityModule*>(qSlicerApplication::application()->moduleManager()->module("virtualreality"));
  assert(module); // dependency system should have assured us this exists

  // Workaround: find view via findChildren approach as module function is not exported
  qSlicerVirtualRealityModuleWidget* widget = dynamic_cast<qSlicerVirtualRealityModuleWidget*>(module->widgetRepresentation());
  assert(widget); // ensure that widget has been created, lazy creation should guarantee this existence

  qMRMLVirtualRealityView* view(nullptr);

  auto list = qSlicerApplication::application()->topLevelWidgets();
  for (auto iter = list.begin(); iter != list.end(); ++iter)
  {
    auto item = *iter;
    if (item->objectName().compare("VirtualRealityWidget") == 0)
    {
      view = dynamic_cast<qMRMLVirtualRealityView*>(item);
      assert(view);

      d->VRView = view;
      d->VRView->renderer()->SetTexturedBackground(true);
      d->VRView->renderer()->SetLeftBackgroundTexture(d->LeftEyeTexture);
      d->VRView->renderer()->SetRightBackgroundTexture(d->RightEyeTexture);
    }
  }
}

//-----------------------------------------------------------------------------
qSlicerVideoPassthroughModuleWidget::~qSlicerVideoPassthroughModuleWidget()
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

}

//----------------------------------------------------------------------------
void qSlicerVideoPassthroughModuleWidget::onLeftEyeNodeChanged(vtkMRMLScalarVolumeNode* node)
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  d->LeftEyeTexture->SetInputDataObject(node->GetImageData());
}

//----------------------------------------------------------------------------
void qSlicerVideoPassthroughModuleWidget::onRightEyeNodeChanged(vtkMRMLScalarVolumeNode* node)
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  d->RightEyeTexture->SetInputDataObject(node->GetImageData());
}

//-----------------------------------------------------------------------------
void qSlicerVideoPassthroughModuleWidget::setup()
{
  Q_D(qSlicerVideoPassthroughModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
