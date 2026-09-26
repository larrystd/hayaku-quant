/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#include "extensions/telemetry/Telemetry.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdio>
#include <mutex>
#include <nlohmann/json.hpp>
#include <shared_mutex>
#include <thread>

#include "common/FileLock.h"
#include "common/Os.h"
#include "extensions/telemetry/HttpClient.h"
#include "version.h"

using json = nlohmann::json;

#define FEEDBACK_SERVER_ADDR "http://hayaku.cpolar.cn"

namespace hayaku {

namespace {

struct TelemetryInfo {
  Datetime expire_time{Datetime::max()};
  LatestVersionInfo latest_version_info;
  std::shared_mutex latest_version_mutex;
};

TelemetryInfo& telemetryInfo() {
  static auto* info = new TelemetryInfo;
  static const bool initialized = [] {
    info->latest_version_info.version = HAYAKU_VERSION_MAJOR * 1000000 +
                                        HAYAKU_VERSION_MINOR * 1000 +
                                        HAYAKU_VERSION_ALTER;
    return true;
  }();
  (void)initialized;
  return *info;
}

boost::uuids::uuid readUUID() {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif
  boost::uuids::uuid uid;
  std::string dir = fmt::format("{}/.hayaku", getUserDir());
  createDir(dir);

  std::string filename = fmt::format("{}/uid", dir);

  auto try_read = [&filename](boost::uuids::uuid& out) -> bool {
    if (!existFile(filename)) {
      return false;
    }
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp) {
      return false;
    }
    bool ok = (fread((void*)out.data, 1, 16, fp) == 16);
    fclose(fp);
    if (!ok) {
      out = boost::uuids::nil_uuid();
    }
    return ok;
  };

  if (!try_read(uid)) {
    FileLock lock(fmt::format("{}.lock", filename));
    if (!lock.waitLock(100, 100)) {
      return uid;
    }

    if (!try_read(uid)) {
      boost::uuids::uuid new_uid = boost::uuids::random_generator()();
      std::string tmp_filename =
          fmt::format("{}.tmp.{}", filename, boost::uuids::to_string(new_uid));
      FILE* fp = fopen(tmp_filename.c_str(), "wb");
      if (fp) {
        size_t n = fwrite(new_uid.data, 16, 1, fp);
        fflush(fp);
        fclose(fp);
        if (n == 1 &&
            std::rename(tmp_filename.c_str(), filename.c_str()) == 0) {
          uid = new_uid;
        } else {
          std::remove(tmp_filename.c_str());
        }
      }
    }
  }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

  return uid;
}

}  // namespace

bool HAYAKU_API CanUpgrade() {
  int current_version = HAYAKU_VERSION_MAJOR * 1000000 +
                        HAYAKU_VERSION_MINOR * 1000 + HAYAKU_VERSION_ALTER;
  auto& info = telemetryInfo();
  std::shared_lock<std::shared_mutex> lock(info.latest_version_mutex);
  return info.latest_version_info.version > current_version;
}

LatestVersionInfo HAYAKU_API getLatestVersionInfo() {
  auto& info = telemetryInfo();
  std::shared_lock<std::shared_mutex> lock(info.latest_version_mutex);
  return info.latest_version_info;
}

void updateSysInfoExpiredTime(Datetime time) {
  telemetryInfo().expire_time = time;
}

void HAYAKU_API reminderLicenseExpiration() {
  auto remain = telemetryInfo().expire_time - Datetime::now();
  HAYAKU_WARN_IF(remain > Days(0) && remain < Days(10),
                 "Note! Your license will expire in {} days.", remain.days());
}

void sendFeedback() {
  std::thread t([] {
    try {
      boost::uuids::uuid uid = readUUID();
      HAYAKU_IF_RETURN(uid.is_nil(), void());

      AsioHttpClient client(FEEDBACK_SERVER_ADDR, 2000);
      json req;
      req["uid"] = boost::uuids::to_string(uid);
      req["part"] = "hayaku";
      req["version"] = HAYAKU_VERSION;
      req["build"] = fmt::format("{}", HAYAKU_VERSION_BUILD);
      req["platform"] = getPlatform();
      req["arch"] = getCpuArch();
      auto res = client.post("/hayaku/visit", req);
      json r = res.json();
      const json& data = r["data"];

      auto& info = telemetryInfo();
      std::unique_lock<std::shared_mutex> lock(info.latest_version_mutex);
      info.latest_version_info.version = data["last_version"].get<int>();
      if (data.contains("remark")) {
        info.latest_version_info.remark = data["remark"].get<std::string>();
        info.latest_version_info.release_date =
            Datetime(data["release_date"].get<std::string>());
      } else {
        info.latest_version_info.remark =
            "release note: https://github.com/larrystd/hayaku-quant/releases";
        info.latest_version_info.release_date = Datetime();
      }
    } catch (...) {
    }
  });
  t.detach();
}

void sendPythonVersionFeedBack(int major, int minor, int micro) {
  std::thread t([=]() {
    try {
      AsioHttpClient client(FEEDBACK_SERVER_ADDR, 2000);
      json req;
      req["major"] = major;
      req["minor"] = minor;
      req["micro"] = micro;
      client.post("/hayaku/pyver", req);
    } catch (...) {
    }
  });
  t.detach();
}

}  // namespace hayaku
