/*
 *
 * BigInt
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/09/2016.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <algorithm>
#include <cstdlib>
#include <iostream>

#include <core/math/BigInt.hpp>
#include <core/collections/Collections.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace ledger {
    namespace core {
        const BigInt BigInt::ZERO = BigInt(0);
        const BigInt BigInt::ONE = BigInt(1);
        const BigInt BigInt::TEN = BigInt(10);
        const std::string BigInt::DIGITS = std::string("0123456789abcdefghijklmnopqrstuvwxyz");
        const int BigInt::MIN_RADIX = 2;
        const int BigInt::MAX_RADIX = 36;

        BigInt::BigInt() {
            _bigd = bdNew();
            _negative = false;
        }

        BigInt::BigInt(const BigInt& cpy) {
            _bigd = bdNew();
            _negative = cpy._negative;
            bdSetEqual(_bigd, cpy._bigd);
        }

        BigInt::BigInt(const void *data, size_t length, bool negative) : BigInt() {
            bdConvFromOctets(_bigd, reinterpret_cast<const unsigned char *>(data), length);
            _negative = negative;
        }

        BigInt::BigInt(const std::vector<uint8_t> &data, bool negative)
        : BigInt(data.data(), data.size(), negative){

        }

        BigInt::BigInt(int value)
                : BigInt() {
            bdSetShort(_bigd, (bdigit_t)std::abs(value));
            if (value < 0) {
                _negative = true;
            }
        }

        BigInt::BigInt(unsigned int value) : BigInt() {
            bdSetShort(_bigd, (bdigit_t)value);
            _negative = false;
        }

        BigInt::BigInt(unsigned long long value) : BigInt() {
            auto bytes = endianness::scalar_type_to_array<unsigned long long>(value, endianness::Endianness::BIG);
            bdConvFromOctets(_bigd, reinterpret_cast<const unsigned char *>(bytes), sizeof(unsigned long long));
            std::free(bytes);
            _negative = false;
        }

        BigInt::BigInt(int64_t value) : BigInt() {
            auto bytes = endianness::scalar_type_to_array<int64_t >(std::abs(value), endianness::Endianness::BIG);
            bdConvFromOctets(_bigd, reinterpret_cast<const unsigned char *>(bytes), sizeof(int64_t));
            std::free(bytes);
            _negative = value < 0LL;
        }

        BigInt::BigInt(const std::string& str) : BigInt(str, 10)
        {};

        BigInt& BigInt::assignI64(int64_t value) {
            auto bytes = endianness::scalar_type_to_array<int64_t >(std::abs(value), endianness::Endianness::BIG);
            bdConvFromOctets(_bigd, reinterpret_cast<const unsigned char *>(bytes), sizeof(int64_t));
            std::free(bytes);
            _negative = value < 0LL;
            return *this;
        }

        BigInt::BigInt(const std::string &str, int radix) : BigInt() {
            if (radix == 10) {
                // handle digit representation
                if (!all_digits(str)) {
                    throw std::invalid_argument("Non-numeric base 10 big int");
                }

                _negative = str[0] == '-';
                bdConvFromDecimal(_bigd, str.c_str());
            } else if (radix == 16) {
                bdConvFromHex(_bigd, str.c_str());
            } else {
                throw std::invalid_argument("Cannot handle radix");
            }
        }

        BigInt::~BigInt() {
            if (_bigd != nullptr) {
                bdFree(&_bigd);
            }
        }

        int BigInt::toInt() const {
            return bdToShort(_bigd) * (_negative ? -1 : 1);
        }

        unsigned int BigInt::toUnsignedInt() const {
            return bdToShort(_bigd);
        }

        std::string BigInt::toString() const {
            size_t nchars = bdConvToDecimal(_bigd, NULL, 0);
            auto s = std::shared_ptr<char>(new char[nchars + 1]);
            bdConvToDecimal(_bigd, s.get(), nchars + 1);
            auto out = std::string(s.get());
            if (this->isNegative()) {
                out = "-" + out;
            }
            return out;
        }

        std::string BigInt::toHexString() const {
            size_t nchars = bdConvToHex(_bigd, NULL, 0);
            auto s = std::shared_ptr<char>(new char[nchars + 1]);
            bdConvToHex(_bigd, s.get(), nchars + 1);
            auto out = std::string(s.get());
            if (out.length() % 2 != 0) {
                out = "0" + out;
            }
            return out;
        }

        unsigned long BigInt::getBitSize() const {
            return bdSizeof(_bigd) * sizeof(SimpleInt) * 8;
        }

        BigInt *BigInt::from_hex(const std::string &str) {
            return new BigInt(str, 16);
        }

        BigInt *BigInt::from_dec(const std::string &str) {
            return new BigInt(str, 10);
        }

        BigInt BigInt::operator+(const BigInt &rhs) const {
            if (rhs.isNegative() && !this->isNegative()) {
                return *this - rhs.positive();
            } else if (this->isNegative() && !rhs.isNegative()) {
                return rhs - this->positive();
            }
            BigInt result;
            bdAdd(result._bigd, this->_bigd, rhs._bigd);
            result._negative = rhs.isNegative() && this->isNegative();
            return result;
        }

        BigInt BigInt::operator-(const BigInt &rhs) const {
            if (this->isPositive() && rhs.isNegative()) {
                return *this + rhs.positive();
            } else if (this->isNegative() && rhs.isPositive()) {
                return *this + rhs.negative();
            } else if (rhs > *this) {
                return (rhs - *this).negative();
            }
            BigInt result;
            bdSubtract(result._bigd, this->_bigd, rhs._bigd);
            return result;
        }

        BigInt BigInt::operator*(const BigInt &rhs) const {
            BigInt result;
            bdMultiply(result._bigd, this->_bigd, rhs._bigd);
            result._negative = this->isNegative() != rhs.isNegative();
            return result;
        }

        BigInt BigInt::operator/(const BigInt &rhs) const {
            BigInt result;
            BigInt remainder;
            bdDivide(result._bigd, remainder._bigd, this->_bigd, rhs._bigd);
            result._negative = this->isNegative() != rhs.isNegative();
            return result;
        }

        BigInt BigInt::operator%(const BigInt &rhs) const {
            BigInt result;
            BigInt remainder;
            bdDivide(result._bigd, remainder._bigd, this->_bigd, rhs._bigd);
            remainder._negative = this->isNegative();
            return remainder;
        }

        BigInt &BigInt::operator++() {
            if (this->isNegative()) {
                bdDecrement(_bigd);
            } else {
                bdIncrement(_bigd);
            }
            return *this;
        }

        BigInt BigInt::operator++(int) {
            BigInt temp = *this;
            ++*this;
            return temp;
        }

        BigInt &BigInt::operator--() {
            if (this->isPositive()) {
                bdDecrement(_bigd);
            } else {
                bdIncrement(_bigd);
            }
            return *this;
        }

        BigInt BigInt::operator--(int) {
            BigInt temp = *this;
            ++*this;
            return temp;
        }

        void BigInt::operator=(const BigInt &a) {
            bdSetEqual(_bigd, a._bigd);
            _negative = a._negative;
        }

        bool BigInt::isNegative() const {
            return _negative && !this->isZero();
        }

        bool BigInt::isPositive() const {
            return !_negative || this->isZero();
        }

        bool BigInt::isZero() const {
            return bdIsZero(_bigd) != 0;
        }

        BigInt BigInt::negative() const {
            BigInt result = *this;
            result._negative = true;
            return result;
        }

        BigInt BigInt::positive() const {
            BigInt result = *this;
            result._negative = false;
            return result;
        }

        bool BigInt::operator<(const BigInt &rhs) const {
            if (this->isNegative() && rhs.isPositive()) {
                return true;
            } else if (this->isPositive() && rhs.isNegative()) {
                return false;
            } else if (this->isNegative() && rhs.isNegative()) {
                return bdCompare(this->_bigd, rhs._bigd) == 1;
            }
            return bdCompare(this->_bigd, rhs._bigd) == -1;
        }

        bool BigInt::operator<=(const BigInt &rhs) const {
            if (this->isNegative() && rhs.isPositive()) {
                return true;
            } else if (this->isPositive() && rhs.isNegative()) {
                return false;
            } else if (this->isNegative() && rhs.isNegative()) {
                return bdCompare(this->_bigd, rhs._bigd) >= 0;
            }
            return bdCompare(this->_bigd, rhs._bigd) <= 0;
        }

        bool BigInt::operator==(const BigInt &rhs) const {
            return this->_negative == rhs._negative && bdCompare(this->_bigd, rhs._bigd) == 0;
        }

        bool BigInt::operator>(const BigInt &rhs) const {
            return rhs < *this;
        }

        bool BigInt::operator>=(const BigInt &) const {
            return false;
        }

        BigInt BigInt::powu(unsigned short p) const {
            BigInt result;
            bdPower(result._bigd, _bigd, p);
            result._negative = isNegative() && (p % 2 != 0 || p == 0);
            return result;
        }

        std::vector<uint8_t> BigInt::toByteArray() const {
            size_t nchars = bdConvToOctets(_bigd, NULL, 0);
            std::vector<uint8_t> out = std::vector<uint8_t >(nchars);
            bdConvToOctets(_bigd, reinterpret_cast<unsigned char *>(out.data()), nchars);
            return out;
        }

        uint64_t BigInt::toUint64() const {
            std::vector<uint8_t> result(sizeof(uint64_t));
            bdConvToOctets(_bigd, result.data(), sizeof(uint64_t));
            if (ledger::core::endianness::isSystemLittleEndian()) {
                std::reverse(result.begin(), result.end());
            }
            return reinterpret_cast<uint64_t *>(result.data())[0];
        }

        int64_t BigInt::toInt64() const {
            std::vector<uint8_t> result(sizeof(uint64_t));
            bdConvToOctets(_bigd, result.data(), sizeof(uint64_t));
            if (ledger::core::endianness::isSystemLittleEndian()) {
                std::reverse(result.begin(), result.end());
            }
            return reinterpret_cast<int64_t *>(result.data())[0] * (_negative ? -1 : 1);
        }

        int BigInt::compare(const BigInt &rhs) const {
            if (this->isNegative() && rhs.isPositive()) {
                return -1;
            } else if (this->isPositive() && rhs.isNegative()) {
                return 1;
            } else if (this->isNegative() && rhs.isNegative()) {
                return -bdCompare(this->_bigd, rhs._bigd);
            }
            return bdCompare(this->_bigd, rhs._bigd);
        }

        BigInt BigInt::fromHex(const std::string &str) {
            return BigInt(str, 16);
        }

        BigInt BigInt::fromDecimal(const std::string &str) {
            return BigInt(str, 10);
        }

        BigInt BigInt::fromString(const std::string &str) {
            if (strings::startsWith(str, "0x")) {
                return BigInt::fromHex(str.substr(2, str.length()));
            } else {
                return BigInt::fromDecimal(str);
            }
        }

        BigInt::BigInt(BigInt &&mov) {
            _bigd = mov._bigd;
            _negative = mov._negative;
            mov._bigd = nullptr;
        }

        std::shared_ptr<api::BigInt> BigInt::add(
            std::shared_ptr<api::BigInt> const& i
        ) {
            auto result = this->operator+(*std::dynamic_pointer_cast<BigInt>(i));
            return std::make_shared<BigInt>(result);
        }

        std::shared_ptr<api::BigInt> BigInt::subtract(
            std::shared_ptr<api::BigInt> const& i
        ) {
            auto result = this->operator-(*std::dynamic_pointer_cast<BigInt>(i));
            return std::make_shared<BigInt>(result);
        }

        std::shared_ptr<api::BigInt> BigInt::multiply(
            std::shared_ptr<api::BigInt> const& i
        ) {
            auto result = this->operator*(*std::dynamic_pointer_cast<BigInt>(i));
            return std::make_shared<BigInt>(result);
        }

        std::shared_ptr<api::BigInt> BigInt::divide(
            std::shared_ptr<api::BigInt> const& i
        ) {
            auto result = this->operator/(*std::dynamic_pointer_cast<BigInt>(i));
            return std::make_shared<BigInt>(result);
        }

        std::vector<std::shared_ptr<api::BigInt>> BigInt::divideAndRemainder(
            std::shared_ptr<api::BigInt> const& i
        ) {
            auto res1 = std::make_shared<BigInt>(this->operator/(*std::dynamic_pointer_cast<BigInt>(i)));
            auto res2 = std::make_shared<BigInt>(this->operator+(*std::dynamic_pointer_cast<BigInt>(i)));
            return std::vector<std::shared_ptr<api::BigInt>>({res1, res2});
        }

        std::shared_ptr<api::BigInt> BigInt::pow(int32_t exponent) {
            BigInt x = powu(static_cast<unsigned short>(exponent));
            return std::make_shared<BigInt>(x);
        }

        std::string BigInt::toDecimalString(
            int32_t precision,
            std::string const& decimalSeparator,
            std::string const& thousandSeparator
        ) {
            return "";
        }

        int32_t BigInt::intValue() {
            return toInt();
        }

        int32_t BigInt::compare(std::shared_ptr<api::BigInt> const& i) {
            return compare(*std::dynamic_pointer_cast<BigInt>(i));
        }

        std::string BigInt::toString(int32_t radix) {
            if (radix == 10) {
                return toString();
            } else {
                return toHexString();
            }
        }

        std::shared_ptr<api::BigInt> api::BigInt::fromDecimalString(const std::string &s, int32_t precision,
                                                          const std::string &decimalSeparator) {
            std::string writer;
            std::string decimaleWriter;
            auto hasReachedDecimalPart = false;
            auto d = 0;
            for (auto i = 0; i < s.length(); i++) {
                auto c = s[i];
                if (c >= '0' && c <= '9' && !hasReachedDecimalPart) {
                    writer.push_back(c);
                } else if (c == decimalSeparator[0] && !hasReachedDecimalPart) {
                    hasReachedDecimalPart = true;
                } else if (c >= '0' && c <= '9' && hasReachedDecimalPart) {
                    decimaleWriter.push_back(c);
                } else {
                    d += 1;
                }
            }
            while (decimaleWriter.size() < precision) {
                decimaleWriter.push_back('0');
            }
            return fromIntegerString(writer + decimaleWriter, 10);
        }

        std::shared_ptr<api::BigInt> api::BigInt::fromIntegerString(const std::string &s, int32_t radix) {
            if (radix == 10) {
                return std::shared_ptr<BigInt>(new ::ledger::core::BigInt(ledger::core::BigInt::fromDecimal(s)));
            } else {
                return std::shared_ptr<BigInt>(new ::ledger::core::BigInt(ledger::core::BigInt::fromHex(s)));
            }
        }
        
        std::shared_ptr<api::BigInt> api::BigInt::fromLong(int64_t value) {
                return std::shared_ptr<BigInt>(new ::ledger::core::BigInt(ledger::core::BigInt::fromScalar(value)));
        }

        bool BigInt::all_digits(std::string const& s) {
            auto it = s.cbegin();
            auto end = s.cend();

            if (it != end && (*it == '-' || *it == '+')) {
                it++; // move past the first character
            }

            return std::all_of(it, end, [](char c) { return isdigit(c); });
        }

        BigInt BigInt::fromFloatString(const std::string &str, int scaleFactor) {
            namespace mp = boost::multiprecision;

            mp::cpp_dec_float_50 f(str);
            mp::cpp_dec_float_50 scale = mp::pow(mp::cpp_dec_float_50(10), (float) scaleFactor);
            f = f * scale;

            bool isNegative = f < 0;

            if (isNegative)
                f = f * -1;

            mp::uint256_t i;
            i.assign(f);

            auto size = i.backend().size();
            auto *limbs = i.backend().limbs();

            // Here is the weird part. Boost is dividing the number into "limbs", the limbs size may differ depending on the
            // platform but there are always ordered like little endian (the least significant limb is always first).
            // We can't forget that limb endianness depends on the architecture. To avoid having to perform multiple
            // swaps for every architecture we forces the whole limb "byte array" to be ordered like a bit uint256 in little endian
            // number and then swap all of it in big endian.
            if (endianness::getSystemEndianness() == endianness::Endianness::BIG) {
                for (auto offset = 0; offset < size; offset++) {
                    endianness::swapToEndianness(limbs + offset, sizeof(mp::limb_type),
                                                 endianness::getSystemEndianness(), endianness::Endianness::LITTLE);
                }
            }
            endianness::swapToEndianness(limbs, size * sizeof(mp::limb_type), endianness::Endianness::LITTLE, endianness::Endianness::BIG);
            return BigInt(limbs, size * sizeof(mp::limb_type), isNegative);
        }
    }
}
