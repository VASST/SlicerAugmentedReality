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

// TrackedScreenAR Logic includes
#include <vtkSlicerTrackedScreenARLogic.h>

// TrackedScreenAR includes
#include "qSlicerTrackedScreenARModule.h"
#include "qSlicerTrackedScreenARModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerTrackedScreenARModulePrivate
{
public:
  qSlicerTrackedScreenARModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTrackedScreenARModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTrackedScreenARModulePrivate::qSlicerTrackedScreenARModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTrackedScreenARModule methods

//-----------------------------------------------------------------------------
qSlicerTrackedScreenARModule::qSlicerTrackedScreenARModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTrackedScreenARModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTrackedScreenARModule::~qSlicerTrackedScreenARModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTrackedScreenARModule::helpText() const
{
  return "This extension supports a workflow for performing screen-based AR where the camera is driven by a transform. Typically, the transform would be provided by some sort of tracking system in conjunction with the SlicerOpenIGTLink extension.";
}

//-----------------------------------------------------------------------------
QString qSlicerTrackedScreenARModule::acknowledgementText() const
{
  return "This work was partially funded by BrainsCAN and the Canadian Foundation for Innovation.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTrackedScreenARModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Adam Rankin (Robarts Research Institute)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerTrackedScreenARModule::icon() const
{
  return QIcon(":/Icons/TrackedScreenAR.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTrackedScreenARModule::categories() const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTrackedScreenARModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerTrackedScreenARModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerTrackedScreenARModule
::createWidgetRepresentation()
{
  return new qSlicerTrackedScreenARModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTrackedScreenARModule::createLogic()
{
  return vtkSlicerTrackedScreenARLogic::New();
}
