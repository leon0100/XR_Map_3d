#include "device_manager_wrapper.h"
// #include "device_defs.h"


DeviceManagerWrapper::DeviceManagerWrapper(QObject* parent) :
    QObject(parent),
    averageChartLosses_(0),
    protoBinConsoledState_(false),
    USBLBeaconDirectAskState_(false)
{
    workerObject_ = std::make_unique<DeviceManager>();
    auto ct = Qt::DirectConnection;
    QObject::connect(this, &DeviceManagerWrapper::sendOpenFile_CSV,  workerObject_.get(), &DeviceManager::openFile_CSV, Qt::QueuedConnection);
    QObject::connect(this, &DeviceManagerWrapper::sendOpenFile_tsl,  workerObject_.get(), &DeviceManager::openFile_tsl, Qt::QueuedConnection);


    QObject::connect(this, &DeviceManagerWrapper::sendCloseFile, workerObject_.get(), &DeviceManager::closeFile,   ct);
    QObject::connect(workerObject_.get(), &DeviceManager::streamChanged,  this, &DeviceManagerWrapper::streamChanged,  ct);
    QObject::connect(workerObject_.get(), &DeviceManager::vruChanged,   this,   &DeviceManagerWrapper::vruChanged,   ct);
    QObject::connect(workerObject_.get(), &DeviceManager::chartLossesChanged,  this,  &DeviceManagerWrapper::calcAverageChartLosses, ct);
}

DeviceManagerWrapper::~DeviceManagerWrapper()
{

}

DeviceManager* DeviceManagerWrapper::getWorker()
{
    return workerObject_.get();
}

QUuid DeviceManagerWrapper::getFileUuid() const
{
    constexpr auto kFileUuidStr = "12345678-1234-1234-1234-1234567890ab";
    return QUuid(kFileUuidStr);
}

void DeviceManagerWrapper::initStreamList()
{
    workerObject_->initStreamList();
}

void DeviceManagerWrapper::resetFileAndChannelId()
{
    workerObject_->resetFileAndChannelId();
}


void DeviceManagerWrapper::calcAverageChartLosses()
{
    // averageChartLosses_ = std::max(0, std::min(100, 100 - getWorker()->calcAverageChartLosses()));
    emit this->chartLossesChanged();
}
