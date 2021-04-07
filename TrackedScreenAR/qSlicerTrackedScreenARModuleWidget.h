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

#ifndef __qSlicerTrackedScreenARModuleWidget_h
#define __qSlicerTrackedScreenARModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerTrackedScreenARModuleExport.h"

class qSlicerTrackedScreenARModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_TRACKEDSCREENAR_EXPORT qSlicerTrackedScreenARModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTrackedScreenARModuleWidget(QWidget* parent = 0);
  virtual ~qSlicerTrackedScreenARModuleWidget();

public slots:
  void onCameraTransformNodeChanged(const QString& nodeId);
  void onVideoSourceNodeChanged(const QString& nodeId);
  void onVideoSourceParametersNodeChanged(const QString& nodeId);

protected:
  void onImageDataModified();

protected:
  QScopedPointer<qSlicerTrackedScreenARModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerTrackedScreenARModuleWidget);
  Q_DISABLE_COPY(qSlicerTrackedScreenARModuleWidget);
};

#endif
