#pragma once

#include <cstdio>
#include <string>

// Tien ich test toi gian, khong phu thuoc framework ngoai.
// Moi bai test goi testx::check(...) nhieu lan, cuoi cung goi testx::summary().
namespace testx {

inline int g_checks = 0;    // tong so lan kiem tra
inline int g_failures = 0;  // so lan kiem tra that bai

// In [ ok ] hoac [FAIL] kem mo ta; tang bo dem tuong ung.
inline void check(bool condition, const std::string& message) {
    ++g_checks;
    if (condition) {
        std::printf("[ ok ] %s\n", message.c_str());
    } else {
        ++g_failures;
        std::fprintf(stderr, "[FAIL] %s\n", message.c_str());
    }
}

// In tong ket va tra ma thoat: 0 = tat ca pass, 1 = co loi.
inline int summary(const char* name) {
    std::printf("%s: %d/%d checks passed\n",
                name, g_checks - g_failures, g_checks);
    return g_failures == 0 ? 0 : 1;
}

}  // namespace testx
