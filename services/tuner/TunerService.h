/**
 * Copyright (c) 2020, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_MEDIA_TUNERSERVICE_H
#define ANDROID_MEDIA_TUNERSERVICE_H

#include <aidl/android/media/tv/tuner/BnTunerService.h>
#include <android/hardware/tv/tuner/1.0/ITuner.h>
#include <fmq/AidlMessageQueue.h>
#include <fmq/EventFlag.h>
#include <fmq/MessageQueue.h>

using ::aidl::android::hardware::common::fmq::GrantorDescriptor;
using ::aidl::android::hardware::common::fmq::MQDescriptor;
using ::aidl::android::hardware::common::fmq::SynchronizedReadWrite;
using ::aidl::android::media::tv::tuner::BnTunerService;
using ::aidl::android::media::tv::tuner::ITunerFrontend;
using ::aidl::android::media::tv::tuner::ITunerLnb;
using ::aidl::android::media::tv::tuner::TunerFrontendInfo;

using ::android::hardware::details::logError;
using ::android::hardware::EventFlag;
using ::android::hardware::kSynchronizedReadWrite;
using ::android::hardware::MessageQueue;
using ::android::hardware::MQDescriptorSync;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::tv::tuner::V1_0::DemuxFilterAvSettings;
using ::android::hardware::tv::tuner::V1_0::DemuxFilterEvent;
using ::android::hardware::tv::tuner::V1_0::DemuxFilterMainType;
using ::android::hardware::tv::tuner::V1_0::DemuxFilterSettings;
using ::android::hardware::tv::tuner::V1_0::DemuxFilterStatus;
using ::android::hardware::tv::tuner::V1_0::DemuxFilterType;
using ::android::hardware::tv::tuner::V1_0::DemuxTsFilterSettings;
using ::android::hardware::tv::tuner::V1_0::DemuxTsFilterType;
using ::android::hardware::tv::tuner::V1_0::FrontendId;
using ::android::hardware::tv::tuner::V1_0::FrontendInfo;
using ::android::hardware::tv::tuner::V1_0::IDemux;
using ::android::hardware::tv::tuner::V1_0::IFilter;
using ::android::hardware::tv::tuner::V1_0::IFilterCallback;
using ::android::hardware::tv::tuner::V1_0::ITuner;
using ::android::hardware::tv::tuner::V1_0::Result;

using Status = ::ndk::ScopedAStatus;

using namespace std;

namespace android {

typedef enum {
    FRONTEND,
    LNB,
    DEMUX,
    DESCRAMBLER,
} TunerResourceType;

struct FilterCallback : public IFilterCallback {
    ~FilterCallback() {}
    Return<void> onFilterEvent(const DemuxFilterEvent&) {
        return Void();
    }
    Return<void> onFilterStatus(const DemuxFilterStatus) {
        return Void();
    }
};

class TunerService : public BnTunerService {
    typedef AidlMessageQueue<int8_t, SynchronizedReadWrite> AidlMessageQueue;
    typedef MessageQueue<uint8_t, kSynchronizedReadWrite> HidlMessageQueue;
    typedef MQDescriptor<int8_t, SynchronizedReadWrite> AidlMQDesc;

public:
    static char const *getServiceName() { return "media.tuner"; }
    static void instantiate();
    TunerService();
    virtual ~TunerService();

    // TODO: create a map between resource id and handles.
    static int getResourceIdFromHandle(int resourceHandle, int /*type*/) {
        return (resourceHandle & 0x00ff0000) >> 16;
    }

    Status getFrontendIds(vector<int32_t>* ids, int32_t* _aidl_return) override;
    Status getFrontendInfo(int32_t frontendHandle, TunerFrontendInfo* _aidl_return) override;
    Status openFrontend(
            int32_t frontendHandle, shared_ptr<ITunerFrontend>* _aidl_return) override;
    Status getFmqSyncReadWrite(
            MQDescriptor<int8_t, SynchronizedReadWrite>* mqDesc, bool* _aidl_return) override;
    Status openLnb(int lnbHandle, shared_ptr<ITunerLnb>* _aidl_return) override;
    Status openLnbByName(const string& lnbName, shared_ptr<ITunerLnb>* _aidl_return) override;

private:
    template <typename HidlPayload, typename AidlPayload, typename AidlFlavor>
    bool unsafeHidlToAidlMQDescriptor(
            const hardware::MQDescriptor<HidlPayload, FlavorTypeToValue<AidlFlavor>::value>& hidl,
            MQDescriptor<AidlPayload, AidlFlavor>* aidl);

    bool getITuner();
    Result openFilter();
    Result openDemux();
    Result configFilter();

    sp<ITuner> mTuner;
    sp<IDemux> mDemux;
    sp<IFilter> mFilter;
    AidlMessageQueue* mAidlMq;
    MQDescriptorSync<uint8_t> mFilterMQDesc;
    AidlMQDesc mAidlMQDesc;
    EventFlag* mEventFlag;
    TunerFrontendInfo convertToAidlFrontendInfo(FrontendInfo halInfo);
};

} // namespace android

#endif // ANDROID_MEDIA_TUNERSERVICE_H
