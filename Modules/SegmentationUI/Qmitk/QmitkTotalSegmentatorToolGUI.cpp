#include "QmitkTotalSegmentatorToolGUI.h"

#include "mitkProcessExecutor.h"
#include "mitkTotalSegmentatorTool.h"
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QIcon>
#include <QMessageBox>
#include <QUrl>
#include <QtGlobal>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkTotalSegmentatorToolGUI, "")

QmitkTotalSegmentatorToolGUI::QmitkTotalSegmentatorToolGUI()
  : QmitkMultiLabelSegWithPreviewToolGUIBase(), m_SuperclassEnableConfirmSegBtnFnc(m_EnableConfirmSegBtnFnc)
{
  // Nvidia-smi command returning zero doesn't always imply lack of GPUs.
  // Pytorch uses its own libraries to communicate to the GPUs. Hence, only a warning can be given.
  if (m_GpuLoader.GetGPUCount() == 0)
  {
    std::string warning = "WARNING: No GPUs were detected on your machine. The TotalSegmentator tool can be very slow.";
    this->ShowErrorMessage(warning);
  }
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  { return !m_FirstPreviewComputation ? m_SuperclassEnableConfirmSegBtnFnc(enabled) : false; };
}

void QmitkTotalSegmentatorToolGUI::ConnectNewTool(mitk::SegWithPreviewTool *newTool)
{
  Superclass::ConnectNewTool(newTool);
  newTool->IsTimePointChangeAwareOff();
  m_FirstPreviewComputation = true;
}

void QmitkTotalSegmentatorToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  m_Controls.setupUi(this);
  this->AutoParsePythonPaths();
#ifndef _WIN32
  m_Controls.sysPythonComboBox->addItem("/usr/bin");
#endif
  m_Controls.sysPythonComboBox->addItem("Select");
  m_Controls.pythonEnvComboBox->addItem("Select");
  m_Controls.pythonEnvComboBox->setDuplicatesEnabled(false);
  m_Controls.pythonEnvComboBox->setDisabled(true);
  m_Controls.previewButton->setDisabled(true);
  m_Controls.statusLabel->setTextFormat(Qt::RichText);
  m_Controls.subtaskComboBox->addItems(m_VALID_TASKS);
  QString welcomeText;
  this->SetGPUInfo();
  if (m_GpuLoader.GetGPUCount() != 0)
  {
    welcomeText = "<b>STATUS: </b><i>Welcome to Total Segmentator tool. You're in luck: " +
                               QString::number(m_GpuLoader.GetGPUCount()) + " GPU(s) were detected.</i>";
  }
  else
  {
    welcomeText = "<b>STATUS: </b><i>Welcome to Total Segmentator tool. Sorry, " +
                              QString::number(m_GpuLoader.GetGPUCount()) + " GPUs were detected.</i>";
  }
  mainLayout->addLayout(m_Controls.verticalLayout);

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));
  connect(m_Controls.installButton, SIGNAL(clicked()), this, SLOT(OnInstallBtnClicked()));
  connect(m_Controls.overrideBox, SIGNAL(stateChanged(int)), this, SLOT(OnOverrideChecked(int)));
  connect(m_Controls.pythonEnvComboBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnPythonPathChanged(m_Controls.pythonEnvComboBox->itemText(index)); });

  Superclass::InitializeUI(mainLayout);
  //QString lastSelectedPyEnv = m_Settings.value("TotalSeg/LastPythonPath").toString();
  //m_Controls.pythonEnvComboBox->insertItem(0, lastSelectedPyEnv);
  const QString storageDir = m_Installer.GetVirtualEnvPath();
  m_IsInstalled = this->IsTotalSegmentatorInstalled(storageDir);
  if (m_IsInstalled)
  {
    m_PythonPath = GetPythonPathFromUI(storageDir);
    m_Installer.SetVirtualEnvPath(m_PythonPath);
    this->EnableAll(m_IsInstalled);
    welcomeText += " Totalsegmentator is already found installed."; 
  }
  else
  {
    welcomeText += " Totalsegmentator not installed. Please click on \"Install Totalsegmentator\" above.";
  }
  this->WriteStatusMessage(welcomeText);
}

void QmitkTotalSegmentatorToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
}

void QmitkTotalSegmentatorToolGUI::SetGPUInfo()
{
  std::vector<QmitkGPUSpec> specs = m_GpuLoader.GetAllGPUSpecs();
  for (const QmitkGPUSpec &gpuSpec : specs)
  {
    m_Controls.gpuComboBox->addItem(QString::number(gpuSpec.id) + ": " + gpuSpec.name + " (" + gpuSpec.memory + ")");
  }
  if (specs.empty())
  {
    m_Controls.gpuComboBox->setEditable(true);
    m_Controls.gpuComboBox->addItem(QString::number(0));
    m_Controls.gpuComboBox->setValidator(new QIntValidator(0, 999, this));
  }
}

unsigned int QmitkTotalSegmentatorToolGUI::FetchSelectedGPUFromUI()
{
  QString gpuInfo = m_Controls.gpuComboBox->currentText();
  if (m_GpuLoader.GetGPUCount() == 0)
  {
    return static_cast<unsigned int>(gpuInfo.toInt());
  }
  else
  {
    QString gpuId = gpuInfo.split(":", QString::SplitBehavior::SkipEmptyParts).first();
    return static_cast<unsigned int>(gpuId.toInt());
  }
}

void QmitkTotalSegmentatorToolGUI::EnableAll(bool isEnable)
{
  m_Controls.previewButton->setEnabled(isEnable);
  m_Controls.subtaskComboBox->setEnabled(isEnable);
  m_Controls.installButton->setEnabled((!isEnable));
}

void QmitkTotalSegmentatorToolGUI::OnInstallBtnClicked()
{
  bool isInstalled = false;
  MITK_INFO << m_Controls.sysPythonComboBox->currentText().toStdString();
  QString systemPython = GetPythonPathFromUI(m_Controls.sysPythonComboBox->currentText());
  MITK_INFO << systemPython.toStdString();
  if (systemPython.isEmpty())
  {
    this->WriteErrorMessage("Couldn't find Python");
    return;
  }
  else
  {
    m_Installer.SetSystemPythonPath(systemPython);
  }
  
  isInstalled = m_Installer.SetupVirtualEnv(m_Installer.m_VENV_NAME);
  if (isInstalled)
  {
    const QString pythonPath = m_Installer.GetVirtualEnvPath();
    m_PythonPath = GetPythonPathFromUI(m_Installer.GetVirtualEnvPath());
    this->WriteStatusMessage("Successfully installed TotalSegmentator");
  }
  else
  {
    this->WriteErrorMessage("Couldn't find TotalSegmentator");
  }
  this->EnableAll(isInstalled);
}

void QmitkTotalSegmentatorToolGUI::OnPreviewBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::TotalSegmentatorTool>();
  if (nullptr == tool)
  {
    return;
  }
  // QString pythonPathTextItem = "";
  try
  {
    m_Controls.previewButton->setEnabled(false);
    qApp->processEvents();
    if (!this->IsTotalSegmentatorInstalled(m_PythonPath))
    {
      throw std::runtime_error(m_WARNING_TOTALSEG_NOT_FOUND);
    }
    bool isFast = m_Controls.fastBox->isChecked();
    QString subTask = m_Controls.subtaskComboBox->currentText();
    if (subTask != m_VALID_TASKS[0])
    {
      isFast = true;
    }
    tool->SetPythonPath(m_PythonPath.toStdString());
    tool->SetGpuId(FetchSelectedGPUFromUI());
    tool->SetFast(isFast);
    tool->SetSubTask(subTask.toStdString());
    this->WriteStatusMessage(QString("<b>STATUS: </b><i>Starting Segmentation task... This might take a while.</i>"));
    tool->UpdatePreview();
    m_Controls.previewButton->setEnabled(true);
  }
  catch (const std::exception &e)
  {
    std::stringstream errorMsg;
    errorMsg << "<b>STATUS: </b>Error while processing parameters for TotalSegmentator segmentation. Reason: "
             << e.what();
    this->ShowErrorMessage(errorMsg.str());
    this->WriteErrorMessage(QString::fromStdString(errorMsg.str()));
    m_Controls.previewButton->setEnabled(true);
    return;
  }
  catch (...)
  {
    std::string errorMsg = "Unkown error occured while generation TotalSegmentator segmentation.";
    this->ShowErrorMessage(errorMsg);
    m_Controls.previewButton->setEnabled(true);
    return;
  }
  this->SetLabelSetPreview(tool->GetPreviewSegmentation());
  tool->IsTimePointChangeAwareOn();
  this->ActualizePreviewLabelVisibility();
  this->WriteStatusMessage("<b>STATUS: </b><i>Segmentation task finished successfully.</i>");
  /* if (!pythonPathTextItem.isEmpty()) // only cache if the prediction ended without errors.
  {
    QString lastSelectedPyEnv = m_Settings.value("TotalSeg/LastPythonPath").toString();
    if (lastSelectedPyEnv != pythonPathTextItem)
    {
      m_Settings.setValue("TotalSeg/LastPythonPath", pythonPathTextItem);
    }
  }*/
}

void QmitkTotalSegmentatorToolGUI::ShowErrorMessage(const std::string &message, QMessageBox::Icon icon)
{
  this->setCursor(Qt::ArrowCursor);
  QMessageBox *messageBox = new QMessageBox(icon, nullptr, message.c_str());
  messageBox->exec();
  delete messageBox;
  MITK_WARN << message;
}

void QmitkTotalSegmentatorToolGUI::WriteStatusMessage(const QString &message)
{
  m_Controls.statusLabel->setText(message);
  m_Controls.statusLabel->setStyleSheet("font-weight: bold; color: white");
}

void QmitkTotalSegmentatorToolGUI::WriteErrorMessage(const QString &message)
{
  m_Controls.statusLabel->setText(message);
  m_Controls.statusLabel->setStyleSheet("font-weight: bold; color: red");
}

bool QmitkTotalSegmentatorToolGUI::IsTotalSegmentatorInstalled(const QString &pythonPath)
{
  QString fullPath = pythonPath;
  fullPath = fullPath.mid(fullPath.indexOf(" ") + 1);
  bool isPythonExists = false;
#ifdef _WIN32
  isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
  if (!(fullPath.endsWith("Scripts", Qt::CaseInsensitive) || fullPath.endsWith("Scripts/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("Scripts");
    isPythonExists =
      (!isPythonExists) ? QFile::exists(fullPath + QDir::separator() + QString("python.exe")) : isPythonExists;
  }
#else
  isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python3"));
  if (!(fullPath.endsWith("bin", Qt::CaseInsensitive) || fullPath.endsWith("bin/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("bin");
    isPythonExists =
      (!isPythonExists) ? QFile::exists(fullPath + QDir::separator() + QString("python3")) : isPythonExists;
  }
#endif
  bool isExists = QFile::exists(fullPath + QDir::separator() + QString("TotalSegmentator")) && isPythonExists;
  return isExists;
}

void QmitkTotalSegmentatorToolGUI::AutoParsePythonPaths()
{
  QString homeDir = QDir::homePath();
  std::vector<QString> searchDirs;
#ifdef _WIN32
  searchDirs.push_back(QString("C:") + QDir::separator() + QString("ProgramData") + QDir::separator() +
                       QString("anaconda3"));
#else
  // Add search locations for possible standard python paths here
  searchDirs.push_back(homeDir + QDir::separator() + "environments");
  searchDirs.push_back(homeDir + QDir::separator() + "anaconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "miniconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "miniconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "anaconda3");
#endif
  for (QString searchDir : searchDirs)
  {
    if (searchDir.endsWith("anaconda3", Qt::CaseInsensitive))
    {
      if (QDir(searchDir).exists())
      {
        m_Controls.sysPythonComboBox->insertItem(0, "(base): " + searchDir);
        searchDir.append((QDir::separator() + QString("envs")));
      }
    }
    for (QDirIterator subIt(searchDir, QDir::AllDirs, QDirIterator::NoIteratorFlags); subIt.hasNext();)
    {
      subIt.next();
      QString envName = subIt.fileName();
      if (!envName.startsWith('.')) // Filter out irrelevent hidden folders, if any.
      {
        m_Controls.sysPythonComboBox->insertItem(0, "(" + envName + "): " + subIt.filePath());
      }
    }
  }
}

void QmitkTotalSegmentatorToolGUI::OnPythonPathChanged(const QString &pyEnv)
{
  if (pyEnv == QString("Select"))
  {
    m_Controls.previewButton->setDisabled(true);
    QString path =
      QFileDialog::getExistingDirectory(m_Controls.pythonEnvComboBox->parentWidget(), "Python Path", "dir");
    if (!path.isEmpty())
    {
      this->OnPythonPathChanged(path);                                  // recall same function for new path validation
      bool oldState = m_Controls.pythonEnvComboBox->blockSignals(true); // block signal firing while inserting item
      m_Controls.pythonEnvComboBox->insertItem(0, path);
      m_Controls.pythonEnvComboBox->setCurrentIndex(0);
      m_Controls.pythonEnvComboBox->blockSignals(
        oldState); // unblock signal firing after inserting item. Remove this after Qt6 migration
    }
  }
  else if (!this->IsTotalSegmentatorInstalled(pyEnv))
  {
    this->ShowErrorMessage(m_WARNING_TOTALSEG_NOT_FOUND);
    m_Controls.previewButton->setDisabled(true);
  }
  else
  {// Show positive status meeage
    m_Controls.previewButton->setDisabled(false);
    m_PythonPath = GetPythonPathFromUI(pyEnv);
  }
}

QString QmitkTotalSegmentatorToolGUI::GetPythonPathFromUI(const QString &pyEnv)
{
  QString pythonPath;
  QString fullPath = pyEnv.mid(pyEnv.indexOf(" ") + 1);
  if (fullPath.isEmpty())
  {
    return fullPath;
  }
  bool isPythonExists = false;
#ifdef _WIN32
  isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
  if (isPythonExists)
  {
    pythonPath = fullPath;
  }
  else if (!(fullPath.endsWith("Scripts", Qt::CaseInsensitive) || fullPath.endsWith("Scripts/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("Scripts");
    pythonPath = fullPath;
  }
#else
  isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python3"));
  if (isPythonExists)
  {
    pythonPath = fullPath;
  }
  else if (!(fullPath.endsWith("bin", Qt::CaseInsensitive) || fullPath.endsWith("bin/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("bin");
    pythonPath = fullPath;
  }
#endif
  return pythonPath;
}

void QmitkTotalSegmentatorToolGUI::OnOverrideChecked(int state)
{
  bool isEnabled = false;
  if (state == Qt::Checked)
  {
    isEnabled = true;
    m_Controls.previewButton->setDisabled(true);
    m_PythonPath.clear();
  }
  else
  {
    m_PythonPath.clear();
    m_Controls.previewButton->setDisabled(true);
    if (m_IsInstalled)
    {
      m_PythonPath = m_Installer.GetVirtualEnvPath();
      this->EnableAll(m_IsInstalled);
    }
  }
  m_Controls.pythonEnvComboBox->setEnabled(isEnabled);
}


bool QmitkTotalSegmentatorToolInstaller::SetupVirtualEnv(const QString& venvName)
{
  MITK_INFO << GetSystemPythonPath().toStdString();
  if (GetSystemPythonPath().isEmpty())
  {
    return false;
  }
  QDir folderPath(GetBaseDir());
  folderPath.mkdir(venvName);
  if (!folderPath.cd(venvName))
  {
    return false; // Check if directory creation was successful.
  }
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&PrintProcessEvent);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);

  args.push_back("-m");
  args.push_back("venv");
  args.push_back(venvName.toStdString());
#ifdef _WIN32
  QString pythonFile = GetSystemPythonPath() + QDir::separator() + "python.exe";
  spExec->Execute(GetBaseDir().toStdString(), pythonFile.toStdString(), args); // Setup local virtual environment
  QString pythonExeFolder = "Scripts";
#else
  QString pythonFile = m_SysPythonPath + QDir::separator() + "python3";
  spExec->Execute(m_BaseDir.toStdString(), "/usr/bin/python3", args); // Setup local virtual environment
  QString pythonExeFolder = "bin";
#endif

  if (folderPath.cd(pythonExeFolder))
  {
    SetPythonPath(folderPath.absolutePath());
    SetPipPath(folderPath.absolutePath());
    std::string workingDir = GetPythonPath().toStdString();
    MITK_INFO << "workingDir: " << workingDir;
    InstallPytorch(workingDir, &PrintProcessEvent);
    for (auto &package : m_PACKAGES)
    {
      PipInstall(package.toStdString(), &PrintProcessEvent);
    }
    std::string pythonCode; // python syntax to check if torch is installed with CUDA.
    pythonCode.append("import torch;");
    pythonCode.append("print('Pytorch was installed with CUDA') if torch.cuda.is_available() else print('PyTorch was "
                      "installed WITHOUT CUDA');");
    ExecutePython(pythonCode, &PrintProcessEvent);
    return true;
  }
  return false;
}

QString QmitkTotalSegmentatorToolInstaller::GetVirtualEnvPath() 
{
  return m_STORAGE_DIR + m_VENV_NAME;
}