#include "SlaveMode.h"

#include "cbl/cbl.h"
#include "utility/utility.h"
#include "CSlaveModule/CSlaveModule.h"
#include "hmiScreens/hmiScreens.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"

#include "HmiGui/HmiGui/HmiGui.h"
#include "HmiBase/HmiBase.h"
#include "GlobalModule/GlobalModule.h"
#include "../EventDispatcher/EventDispatch.h"
#include "../EventDispatcher/DispEventHandle.h"
#include "../webserverCallback/webserverCallback.h"
#include "../task/common.h"
#include "utility/dir/dir.h"

#include <list>
#ifndef WIN32

#include <dirent.h>

#else

#include <windows.h>

#endif // !WIN32


static std::mutex g_loadLock;
std::vector<string> g_loadImages;
static volatile bool g_bIsConnectMasterHmi = false;
static volatile bool g_bIsGetResouceSucess = false;
static volatile bool g_bIsStartLoad = false;
static volatile bool g_bIsStartInit = false;

int loadMasterResource()
{
#ifndef WIN32
	while (true) {

		if (g_bIsStartInit) {

			return 0;
		}

		if (!g_bIsGetResouceSucess) {

			hmiproto::hmiact dataproto;
			auto redisdata = dataproto.mutable_redis();

			redisdata->set_type(3);
			redisdata->set_topic(CSlaveModule::get_instance().getTopic());
			CSlaveModule::get_instance().sendtoSlaveClient(dataproto, CSlaveModule::get_instance().getTopic() + "/master");
			usleep(1000 * 1000);
			continue;
		}

		if (g_bIsStartLoad) {

			std::vector<std::string> listImages;
			std::string sDevPath = "/mnt/data/img/";
#ifdef WIN32
			sDevPath = slaveResourcePath;
			sDevPath += "img/";
			cbl::removeDir(sDevPath);
#else
			cbl::removeDir(sDevPath, false);
#endif // WIN32


			std::string sUrl = "http://" + CSlaveModule::get_instance().getHost() + "/slaveImg/";
			cbl::CHttpDownload httpC;
			int nRet = -1;
			{
				unique_lock<mutex> mylock(g_loadLock);
				listImages.swap(g_loadImages);
			}
			auto it = listImages.begin();
			for (; it != listImages.end(); it++)
			{
				nRet = httpC.get(sUrl + *it, sDevPath + *it);
				if (0 != nRet) {

					g_bIsGetResouceSucess = false;
				}
			}
			g_bIsStartLoad = false;
			g_bIsStartInit = true;
			break;
		}
	}
#else
	g_bIsStartInit = true;
#endif // !WIN32

	
	return 0;
}

void* runMasterCheckSlavePing(void* arg) {

	if (!CSlaveModule::get_instance().isConnected()) {

		CSlaveModule::get_instance().reconnect();
	}
	//std::vector<int> delClientIds;
	//while (true) {
	//
	//	delClientIds.clear();

	//	CSlaveModule::get_instance().getPingTimeOutClientId(delClientIds);

	//	for (size_t i = 0; i < delClientIds.size(); i++)
	//	{
	//		auto nCliendId = delClientIds[i];
	//		CSecureCtrl::getInstance().destoryUserSecure(nCliendId);
	//		CUserMgr::getInstances().userQuit(nCliendId);
	//		CHMIScreens::get_instance().quitClient(nCliendId);
	//		CEngineMgr::getInstance().resetUserReg(nCliendId);
	//		CSlaveModule::get_instance().deleteSlaveClientById(nCliendId);
	//	}
	usleep(500 * 1000);
	//}

	return nullptr;
}

int redisMasterRecvFunc(const string& sChannel, const string& sMessage, void* lpParameter) {

	//printf("redis master subscribe recv %s:%d\n", sChannel.c_str(), sMessage.size());

	hmiproto::hmiact data;

	google::protobuf::io::ArrayInputStream input(sMessage.data(), sMessage.size());
	google::protobuf::io::GzipInputStream gzip(&input, google::protobuf::io::GzipInputStream::ZLIB);
	// ���л�ʧ��
	if (!data.ParseFromZeroCopyStream(&gzip)) {
		return -1;
	}
	if (data.has_redis()) {

		auto redisData = data.redis();
		if (redisData.has_type()) {

			int nType = redisData.type();
			printf("redis master recv type:%d\n", nType);
			if (0 == nType) { //init

				int nClientId = -1;
				if (redisData.has_topic()) {

					nClientId = CSlaveModule::get_instance().addSlaveClient(redisData);
					CSecureCtrl::getInstance().destoryUserSecure(nClientId);
					CUserMgr::getInstances().userQuit(nClientId);
					CHMIScreens::get_instance().quitClient(nClientId);
					CEngineMgr::getInstance().resetUserReg(nClientId);
				}

				EVENT_T event;
				event.nClientId = nClientId;
				event.nType = EVENT_SCRINIT;
				event.nScreenID = 0;
				event.nConnectType = 2;
				event.sClientTopic = redisData.topic();
				CEventDispatch::getInstance().postEvent(event);
			}
			else if (1 == nType) {

				if (redisData.has_topic()) {

					if (CSlaveModule::get_instance().slaveClientIsExit(redisData.topic())) {

						printf("redis master recv slave ping pack\n");
						CSlaveModule::get_instance().updatePingTime(redisData.topic());
						hmiproto::hmiact dataproto;
						auto redisdata = dataproto.mutable_redis();

						redisdata->set_type(1);
						redisdata->set_topic(CSlaveModule::get_instance().getTopic());

						std::string sData;
						dataproto.SerializeToString(&sData);
						/*CSlaveModule::get_instance().send(redisData.topic() + "/slave", sData);*/
					}
				}
			}
			else if (2 == nType) {

				doRemoteHmiEvent(*data.mutable_event());
			}
			else if (3 == nType) {

				std::string sImageDir = "", fontDir = "", testPiDir = "";

#ifdef WIN32
				getProjectPath(sImageDir);
				sImageDir += "img/";
#else
				sImageDir = getPrjDir() + "img/";
#endif

				std::vector<std::string> filePathList;
				if (UTILITY_NAMESPACE::CDir::listRelativeDir(sImageDir, "*.*", filePathList) < 0) {
					printf("%s:listRelativeDir error\r\n", __FUNCTION__);
				}
				hmiproto::hmiact dataproto;
				auto sendRedisdata = dataproto.mutable_redis();

				sendRedisdata->set_type(3);
				sendRedisdata->set_topic(CSlaveModule::get_instance().getTopic());

				std::string sPrjMd5 = "";
				testPiDir = getPrjDir() + std::string("test.pi");
				cbl::md5File(testPiDir, sPrjMd5);
				sendRedisdata->set_prjmd5(sPrjMd5);

				for (size_t i = 0; i < filePathList.size(); i++)
				{
					if ("." == filePathList[i] || ".." == filePathList[i]) {
					
						continue;
					}
					sendRedisdata->add_imgs(filePathList[i]);
				}

				CSlaveModule::get_instance().sendtoSlaveClient(dataproto, redisData.topic() + "/slave");
			}
			else {

				if (redisData.has_topic()) {

					CSlaveModule::get_instance().slaveClientIsExit(redisData.topic());
				}
			}
		}
	}

	return 0;
}

int redisMasterCloseFunc(void* lpParameter) {

	return 0;
}

int redisSlaveRecvFunc(const string& sChannel, const string& sMessage, void* lpParameter) {
	//cout << "redis slave subscribe recv  " << sChannel.c_str() << ":" << sMessage.size() << endl;

	hmiproto::hmiact dataproto;
	google::protobuf::io::ArrayInputStream input(sMessage.data(), sMessage.size());
	google::protobuf::io::GzipInputStream gzip(&input, google::protobuf::io::GzipInputStream::ZLIB);

	if (!dataproto.ParseFromZeroCopyStream(&gzip)) {

		printf("[%s-%d] parse proto data error\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (dataproto.has_redis() && dataproto.redis().type() == 1) {

		printf("[%s-%d] slave recv protodata has not redis\n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (dataproto.has_redis() && dataproto.redis().type() == 3) {

		g_bIsGetResouceSucess = true;
		std::string sPrjMd5 = "";
		auto redisData = dataproto.redis();
		if (redisData.has_prjmd5()) {
		
			sPrjMd5 = redisData.prjmd5();
		}
		if (CSlaveModule::get_instance().comparePrjMd5(sPrjMd5)) {
		
			g_bIsStartInit = true;
			printf("slave and master project md5 is same\n");
			return 0;
		}

		CSlaveModule::get_instance().updatePrjMd5(sPrjMd5);
		auto images = redisData.imgs();
		unique_lock<mutex> mylock(g_loadLock);
		for (int i = 0; i < redisData.imgs_size(); i++)
		{
			g_loadImages.push_back(images[i]);
		}
		g_bIsStartLoad = true;

		return 0;
	}

	const hmiproto::hmievent& msgEvent = dataproto.event();
	unsigned int nType = msgEvent.type();
	if (nType == HMIEVENT_INITSCRPART) {

		g_bIsConnectMasterHmi = true;
		CSlaveModule::get_instance().setCurrentSlaveClientId(msgEvent.clientid());
	}

	CDispEventHandle::getInstance().sendScreenData(dataproto, QTCLIENT_T);

	return 0;
}

int redisSlaveCloseFunc(void* lpParameter) {
	cout << "slave redis is colse" << endl;
	g_bIsConnectMasterHmi = false;
	g_bIsGetResouceSucess = false;
	CSlaveModule::get_instance().disconnect();
	return 0;
}

void* runConnectToMasterHmi(void* arg) {

	while (true) {
		
		if (!CSlaveModule::get_instance().isConnected()) {

			printf("slave redis client is going to reconnect\n");
			CSlaveModule::get_instance().reconnect();
			usleep(1000 * 1000);
			continue;
		}

		if (HmiGui::getHmiMgrPtr() != nullptr && g_bIsStartInit) {

			if (!g_bIsConnectMasterHmi) {

				static bool bIsFirst = false;
				if (!bIsFirst) {

					bIsFirst = true;
					hmiproto::hmiact dataproto;
					auto redisdata = dataproto.mutable_redis();
					auto slaveInfo = redisdata->mutable_slaveinfo();

					redisdata->set_type(0);
					redisdata->set_topic(CSlaveModule::get_instance().getTopic());

					std::string sProducntId, sProductCode, sProductName, sDeviceIp;
					SECURE::WMachine cMachineT;
					if (cMachineT.getDeviceId(sProducntId) < 0) {
						printf("[%s-%d] start hmi get device id error\n", __FUNCTION__, __LINE__);
					}
					if (!cMachineT.GetProduct(sProducntId)) {
						printf("[%s-%d] start hmi get device code error\n", __FUNCTION__, __LINE__);
					}
					sProductName = cMachineT.getDeviceAlias();

					slaveInfo->set_deviceip(CSlaveModule::get_instance().getCurWorkIp());
					slaveInfo->set_productid(sProducntId);
					slaveInfo->set_productcode(sProductCode);
					slaveInfo->set_productname(sProductName);

					CSlaveModule::get_instance().sendtoSlaveClient(dataproto, CSlaveModule::get_instance().getTopic() + "/master");
				}
			}
			else {

				hmiproto::hmiact dataproto;
				auto redisdata = dataproto.mutable_redis();

				redisdata->set_type(1);
				redisdata->set_topic(CSlaveModule::get_instance().getTopic());

				std::string sData;
				dataproto.SerializeToString(&sData);
				//CSlaveModule::get_instance().send(CSlaveModule::get_instance().getTopic() + "/master", sData);
			}
		}

		usleep(1000 * 1000);
	}

	return nullptr;
}