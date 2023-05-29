// Copyright 2010-2021, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// A utility function that convert string array into exsistence filter
// data file.

#include "rewriter/gen_existence_data.h"

#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>

#include "base/codegen_bytearray_stream.h"
#include "base/hash.h"
#include "base/logging.h"
#include "storage/existence_filter.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"

namespace mozc {
namespace {

using ::mozc::storage::ExistenceFilterBuilder;

std::string GenExistenceData(const absl::Span<const std::string> entries,
                             double error_rate) {
  const int n = entries.size();
  const int m =
      ExistenceFilterBuilder::MinFilterSizeInBytesForErrorRate(error_rate, n);
  LOG(INFO) << "entry: " << n << " err: " << error_rate << " bytes: " << m;

  ExistenceFilterBuilder builder(ExistenceFilterBuilder::CreateOptimal(m, n));

  for (const std::string &entry : entries) {
    const uint64_t id = Hash::Fingerprint(entry);
    builder.Insert(id);
  }
  return builder.SerializeAsString();
}

}  // namespace

void OutputExistenceHeader(const absl::Span<const std::string> entries,
                           const absl::string_view data_namespace,
                           std::ostream *ofs, double error_rate) {
  const std::string existence_data = GenExistenceData(entries, error_rate);

  *ofs << "// This header file is generated by "
       << "gen_existence_data." << std::endl;

  *ofs << "namespace " << data_namespace << "{" << std::endl;

  CodeGenByteArrayOutputStream codegen_stream(ofs,
                                              codegenstream::NOT_OWN_STREAM);
  codegen_stream.OpenVarDef("ExistenceFilter");
  codegen_stream.write(existence_data.data(), existence_data.size());
  codegen_stream.CloseVarDef();
  *ofs << "}  // namespace " << data_namespace << std::endl;
}

void OutputExistenceBinary(const absl::Span<const std::string> entries,
                           std::ostream *ofs, double error_rate) {
  const std::string existence_data = GenExistenceData(entries, error_rate);
  ofs->write(existence_data.data(), existence_data.size());
}
}  // namespace mozc
