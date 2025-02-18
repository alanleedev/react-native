/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "EventBeat.h"

#include <react/debug/react_native_assert.h>
#include <react/renderer/runtimescheduler/RuntimeScheduler.h>
#include <utility>

namespace facebook::react {

EventBeat::EventBeat(
    std::shared_ptr<OwnerBox> ownerBox,
    RuntimeScheduler& runtimeScheduler)
    : ownerBox_(std::move(ownerBox)), runtimeScheduler_(runtimeScheduler) {}

void EventBeat::request() const {
  react_native_assert(
      beatCallback_ &&
      "Unexpected state: EventBeat::setBeatCallback was not called before EventBeat::request.");
  isRequested_ = true;
}

void EventBeat::setBeatCallback(BeatCallback beatCallback) {
  beatCallback_ = std::move(beatCallback);
}

void EventBeat::induce() const {
  if (!isRequested_ || isBeatCallbackScheduled_) {
    return;
  }

  isRequested_ = false;
  isBeatCallbackScheduled_ = true;

  runtimeScheduler_.scheduleWork(
      [this, ownerBox = ownerBox_](jsi::Runtime& runtime) {
        auto owner = ownerBox->owner.lock();
        if (!owner) {
          return;
        }

        isBeatCallbackScheduled_ = false;
        if (beatCallback_) {
          beatCallback_(runtime);
        }
      });
}

} // namespace facebook::react
