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
#include <vtkTrivialProducer.h>

// VTK OpenVR includes
#include <vtkOpenVRRenderer.h>

// Qt includes
#include <QMutex>
#include <QMutexLocker>
#include <QRunnable>
#include <QThreadPool>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_AugmentedReality
class qPollTextureUpdateThread : public QRunnable
{
public:
  //-----------------------------------------------------------------------------
  void run() override
  {
    while (true)
    {
      QMutexLocker(&this->ThreadMutex);

      if (this->ThreadCancelRequested)
      {
        return;
      }

      if (this->LeftEyeNode == nullptr || this->RightEyeNode == nullptr ||
          this->LeftEyeNode->GetImageData() == nullptr || this->RightEyeNode->GetImageData() == nullptr)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        continue;
      }

      // See if modified or image data modified, if new, update textures with new image data
      if (this->LeftEyeNode->GetImageData()->GetMTime() > this->LeftEyeLastMTime || this->LeftEyeModified)
      {
        this->LeftEyeModified = false;
        this->LeftEyeLastMTime = this->LeftEyeNode->GetImageData()->GetMTime();
      }

      if (this->RightEyeNode->GetImageData()->GetMTime() > this->RightEyeLastMTime || this->RightEyeModified)
      {
        this->RightEyeModified = false;
        this->RightEyeLastMTime = this->RightEyeNode->GetImageData()->GetMTime();
      }
    }
  }


  //-----------------------------------------------------------------------------
  qPollTextureUpdateThread::~qPollTextureUpdateThread()
  {
    this->LeftEyeTexture->Delete();
    this->RightEyeTexture->Delete();
  }

  //-----------------------------------------------------------------------------
  void cancelThread()
  {
    QMutexLocker(&this->ThreadMutex);
    this->ThreadCancelRequested = true;
  }

  //-----------------------------------------------------------------------------
  void setLeftEyeNode(vtkMRMLScalarVolumeNode* node)
  {
    QMutexLocker(&this->ThreadMutex);
    this->LeftEyeNode = node;
    this->LeftEyeModified = true;
  }

  //-----------------------------------------------------------------------------
  void setRightEyeNode(vtkMRMLScalarVolumeNode* node)
  {
    QMutexLocker(&this->ThreadMutex);
    this->RightEyeNode = node;
    this->RightEyeModified = true;
  }

  qMRMLVirtualRealityView*  VRView = nullptr;

  bool                      ThreadCancelRequested = false;
  QMutex                    ThreadMutex;

  vtkTexture*               LeftEyeTexture = vtkTexture::New();
  vtkTexture*               RightEyeTexture = vtkTexture::New();
  vtkMRMLScalarVolumeNode*  LeftEyeNode = nullptr;
  vtkMRMLScalarVolumeNode*  RightEyeNode = nullptr;
  vtkMTimeType              LeftEyeLastMTime = 0;
  vtkMTimeType              RightEyeLastMTime = 0;
  bool                      LeftEyeModified = false;
  bool                      RightEyeModified = false;
};

class qTrivialProducerTextureUpdateThread : public QRunnable
{
public:
  //-----------------------------------------------------------------------------
  void run() override
  {
    while (true)
    {
      this->ThreadMutex.lock();
      if (this->ThreadCancelRequested)
      {
        return;
      }

      if (this->LeftEyeNode == nullptr || this->RightEyeNode == nullptr ||
          this->LeftEyeNode->GetImageData() == nullptr || this->RightEyeNode->GetImageData() == nullptr)
      {
        this->ThreadMutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        continue;
      }

      this->ThreadMutex.unlock();
    }
  }

  //-----------------------------------------------------------------------------
  qTrivialProducerTextureUpdateThread::qTrivialProducerTextureUpdateThread()
  {
    this->LeftEyeTexture->SetInputConnection(this->LeftTrivialProducer->GetOutputPort());
    this->RightEyeTexture->SetInputConnection(this->RightTrivialProducer->GetOutputPort());
  }

  //-----------------------------------------------------------------------------
  qTrivialProducerTextureUpdateThread::~qTrivialProducerTextureUpdateThread()
  {
    this->LeftEyeTexture->Delete();
    this->RightEyeTexture->Delete();
  }

  //-----------------------------------------------------------------------------
  void cancelThread()
  {
    ThreadMutex.lock();
    this->ThreadCancelRequested = true;
    this->ThreadMutex.unlock();
  }

  //-----------------------------------------------------------------------------
  void setLeftEyeNode(vtkMRMLScalarVolumeNode* node)
  {
    this->ThreadMutex.lock();

    this->LeftEyeNode = node;
    if(node->GetImageData() == nullptr)
    {
      return;
    }
    this->LeftTrivialProducer->SetInputDataObject(node->GetImageData());
    this->LeftEyeModified = true;

    this->ThreadMutex.unlock();
  }

  //-----------------------------------------------------------------------------
  void setRightEyeNode(vtkMRMLScalarVolumeNode* node)
  {
    this->ThreadMutex.lock();

    this->RightEyeNode = node;
    if (node->GetImageData() == nullptr)
    {
      return;
    }
    this->RightTrivialProducer->SetInputDataObject(node->GetImageData());
    this->RightEyeModified = true;

    this->ThreadMutex.unlock();
  }

  qMRMLVirtualRealityView*  VRView = nullptr;

  bool                      ThreadCancelRequested = false;
  QMutex                    ThreadMutex;

  vtkTexture*               LeftEyeTexture = vtkTexture::New();
  vtkTexture*               RightEyeTexture = vtkTexture::New();
  vtkMRMLScalarVolumeNode*  LeftEyeNode = nullptr;
  vtkMRMLScalarVolumeNode*  RightEyeNode = nullptr;
  vtkTrivialProducer*       LeftTrivialProducer = vtkTrivialProducer::New();
  vtkTrivialProducer*       RightTrivialProducer = vtkTrivialProducer::New();
  vtkMTimeType              LeftEyeLastMTime = 0;
  vtkMTimeType              RightEyeLastMTime = 0;
  bool                      LeftEyeModified = false;
  bool                      RightEyeModified = false;
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_AugmentedReality
class qSlicerVideoPassthroughModuleWidgetPrivate: public Ui_qSlicerVideoPassthroughModuleWidget
{
public:
  qSlicerVideoPassthroughModuleWidgetPrivate();

  qPollTextureUpdateThread TextureUpdateThread;
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

  d->TextureUpdateThread.setAutoDelete(false);

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

      d->TextureUpdateThread.VRView = view;
      d->TextureUpdateThread.VRView->renderer()->SetTexturedBackground(true);
      d->TextureUpdateThread.VRView->renderer()->SetLeftBackgroundTexture(d->TextureUpdateThread.LeftEyeTexture);
      d->TextureUpdateThread.VRView->renderer()->SetRightBackgroundTexture(d->TextureUpdateThread.RightEyeTexture);

      QThreadPool::globalInstance()->start(&d->TextureUpdateThread);
    }
  }
}

//-----------------------------------------------------------------------------
qSlicerVideoPassthroughModuleWidget::~qSlicerVideoPassthroughModuleWidget()
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  d->TextureUpdateThread.cancelThread();
}

//----------------------------------------------------------------------------
void qSlicerVideoPassthroughModuleWidget::onLeftEyeNodeChanged(vtkMRMLScalarVolumeNode* node)
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  d->TextureUpdateThread.ThreadMutex.lock();
  d->TextureUpdateThread.setLeftEyeNode(node);
  d->TextureUpdateThread.ThreadMutex.unlock();
}

//----------------------------------------------------------------------------
void qSlicerVideoPassthroughModuleWidget::onRightEyeNodeChanged(vtkMRMLScalarVolumeNode* node)
{
  Q_D(qSlicerVideoPassthroughModuleWidget);

  d->TextureUpdateThread.ThreadMutex.lock();
  d->TextureUpdateThread.setRightEyeNode(node);
  d->TextureUpdateThread.ThreadMutex.unlock();
}

//-----------------------------------------------------------------------------
void qSlicerVideoPassthroughModuleWidget::setup()
{
  Q_D(qSlicerVideoPassthroughModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
