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

#ifndef __qSlicerVideoPassthroughModuleWidget_h
#define __qSlicerVideoPassthroughModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVideoPassthroughModuleExport.h"

class QMutex;
class qSlicerVideoPassthroughModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLScalarVolumeNode;

/// \ingroup Slicer_QtModules_AugmentedReality
class Q_SLICER_QTMODULES_VIDEOPASSTHROUGH_EXPORT qSlicerVideoPassthroughModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVideoPassthroughModuleWidget(QWidget* parent = 0);
  virtual ~qSlicerVideoPassthroughModuleWidget();

public slots:
  void onLeftEyeNodeChanged(vtkMRMLScalarVolumeNode* node);
  void onRightEyeNodeChanged(vtkMRMLScalarVolumeNode* node);

protected:
  QScopedPointer<qSlicerVideoPassthroughModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerVideoPassthroughModuleWidget);
  Q_DISABLE_COPY(qSlicerVideoPassthroughModuleWidget);
};

#endif
