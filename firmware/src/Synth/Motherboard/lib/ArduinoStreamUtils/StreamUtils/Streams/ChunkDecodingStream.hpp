// StreamUtils - github.com/bblanchon/ArduinoStreamUtils
// Copyright Benoit Blanchon 2019-2024
// MIT License

#pragma once

#include "../Policies/ChunkDecodingPolicy.hpp"
#include "../Policies/WriteForwardingPolicy.hpp"

#include "StreamProxy.hpp"

namespace StreamUtils {

struct ChunkDecodingStream
    : StreamProxy<ChunkDecodingPolicy, WriteForwardingPolicy> {
  ChunkDecodingStream(Stream &target)
      : StreamProxy<ChunkDecodingPolicy, WriteForwardingPolicy>(target) {}

  bool error() const {
    return _reader.error();
  }

  bool ended() const {
    return _reader.ended();
  }
};

}  // namespace StreamUtils
