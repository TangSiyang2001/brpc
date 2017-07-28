// Baidu RPC - A framework to host and access services throughout Baidu.
// Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved

// Author: The baidu-rpc authors (pbrpc@baidu.com)
// Date: 2015/03/20 17:44:29

#include <limits>
#include "base/macros.h"
#include "brpc/details/method_status.h"

namespace brpc {

static int cast_nprocessing(void* arg) {
    return *(int*)arg;
}

MethodStatus::MethodStatus()
    : _nprocessing_bvar(cast_nprocessing, &_nprocessing)
    , _max_concurrency(0)
    , _nprocessing(0) {
}

MethodStatus::~MethodStatus() {
}

int MethodStatus::Expose(const base::StringPiece& prefix) {
    if (_nprocessing_bvar.expose_as(prefix, "processing") != 0) {
        return -1;
    }
    if (_nerror.expose_as(prefix, "error") != 0) {
        return -1;
    }
    if (_latency_rec.expose(prefix) != 0) {
        return -1;
    }
    return 0;
}

template <typename T>
void OutputTextValue(std::ostream& os,
                     const char* prefix,
                     const T& value) {
    os << prefix << value << "\n";
}

template <typename T>
void OutputValue(std::ostream& os,
                 const char* prefix,
                 const std::string& bvar_name,
                 const T& value,
                 const DescribeOptions& options,
                 bool expand) {
    if (options.use_html) {
        os << "<p class=\"variable";
        if (expand) {
            os << " default_expand";
        }
        os << "\">" << prefix << "<span id=\"value-" << bvar_name << "\">"
           << value
           << "</span></p><div class=\"detail\"><div id=\"" << bvar_name
           << "\" class=\"flot-placeholder\"></div></div>\n";
    } else {
        return OutputTextValue(os, prefix, value);
    }
}

void MethodStatus::Describe(
    std::ostream &os, const DescribeOptions& options) const {
    // Sort by alphebetical order to be consistent with /vars.
    const int64_t qps = _latency_rec.qps();
    const bool expand = (qps != 0);
    OutputValue(os, "count: ", _latency_rec.count_name(), _latency_rec.count(),
                options, false);
    OutputValue(os, "error: ", _nerror.name(), _nerror.get_value(),
                options, false);
    OutputValue(os, "latency: ", _latency_rec.latency_name(),
                _latency_rec.latency(), options, false);
    if (options.use_html) {
        OutputValue(os, "latency_percentiles: ",
                    _latency_rec.latency_percentiles_name(),
                    _latency_rec.latency_percentiles(), options, expand);
        OutputValue(os, "latency_cdf: ", _latency_rec.latency_cdf_name(),
                    "click to view", options, expand);
    } else {
        OutputTextValue(os, "latency_50: ",
                        _latency_rec.latency_percentile(0.5));
        OutputTextValue(os, "latency_90: ",
                        _latency_rec.latency_percentile(0.9));
        OutputTextValue(os, "latency_99: ",
                        _latency_rec.latency_percentile(0.99));
        OutputTextValue(os, "latency_999: ",
                        _latency_rec.latency_percentile(0.999));
        OutputTextValue(os, "latency_9999: ",
                        _latency_rec.latency_percentile(0.9999));
    }
    OutputValue(os, "max_latency: ", _latency_rec.max_latency_name(),
                _latency_rec.max_latency(), options, false);
    OutputValue(os, "qps: ", _latency_rec.qps_name(), _latency_rec.qps(),
                options, expand);
    // Many people are confusing with the old name "unresponded" which
    // contains "un" generally associated with something wrong. Name it
    // to "processing" should be more understandable.
    OutputValue(os, "processing: ", _nprocessing_bvar.name(),
                _nprocessing, options, false);
}

}  // namespace brpc