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

// VideoPassthrough Logic includes
#include <vtkSlicerVideoPassthroughLogic.h>

// VideoPassthrough includes
#include "qSlicerVideoPassthroughModule.h"
#include "qSlicerVideoPassthroughModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  #include <QtPlugin>
  Q_EXPORT_PLUGIN2(qSlicerVideoPassthroughModule, qSlicerVideoPassthroughModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerVideoPassthroughModulePrivate
{
public:
  qSlicerVideoPassthroughModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerVideoPassthroughModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerVideoPassthroughModulePrivate::qSlicerVideoPassthroughModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVideoPassthroughModule methods

//-----------------------------------------------------------------------------
qSlicerVideoPassthroughModule::qSlicerVideoPassthroughModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVideoPassthroughModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVideoPassthroughModule::~qSlicerVideoPassthroughModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVideoPassthroughModule::helpText() const
{
  return "This is a loadable module that sets the left and right eye background textures on a OpenVR device";
}

//-----------------------------------------------------------------------------
QString qSlicerVideoPassthroughModule::acknowledgementText() const
{
  return "This work was partially funded by BrainsCAN.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVideoPassthroughModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Adam Rankin (Robarts Research Institute)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerVideoPassthroughModule::icon() const
{
  return QIcon(":/Icons/VideoPassthrough.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVideoPassthroughModule::categories() const
{
  return QStringList() << "Augmented Reality";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVideoPassthroughModule::dependencies() const
{
  return QStringList() << "VirtualReality";
}

//-----------------------------------------------------------------------------
void qSlicerVideoPassthroughModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerVideoPassthroughModule
::createWidgetRepresentation()
{
  return new qSlicerVideoPassthroughModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVideoPassthroughModule::createLogic()
{
  return vtkSlicerVideoPassthroughLogic::New();
}
