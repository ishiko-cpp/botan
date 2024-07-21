/*
* (C) 2024 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/internal/pcurves_instance.h>

#include <botan/internal/pcurves_solinas.h>
#include <botan/internal/pcurves_wrap.h>

namespace Botan::PCurve {

namespace {

namespace sm2p256v1 {

template <typename Params>
class Sm2p256v1Rep final {
   public:
      static constexpr auto P = Params::P;
      static constexpr size_t N = Params::N;
      typedef typename Params::W W;

      constexpr static std::array<W, N> redc(const std::array<W, 2 * N>& z) {
         const int64_t X00 = get_uint32(z.data(), 0);
         const int64_t X01 = get_uint32(z.data(), 1);
         const int64_t X02 = get_uint32(z.data(), 2);
         const int64_t X03 = get_uint32(z.data(), 3);
         const int64_t X04 = get_uint32(z.data(), 4);
         const int64_t X05 = get_uint32(z.data(), 5);
         const int64_t X06 = get_uint32(z.data(), 6);
         const int64_t X07 = get_uint32(z.data(), 7);
         const int64_t X08 = get_uint32(z.data(), 8);
         const int64_t X09 = get_uint32(z.data(), 9);
         const int64_t X10 = get_uint32(z.data(), 10);
         const int64_t X11 = get_uint32(z.data(), 11);
         const int64_t X12 = get_uint32(z.data(), 12);
         const int64_t X13 = get_uint32(z.data(), 13);
         const int64_t X14 = get_uint32(z.data(), 14);
         const int64_t X15 = get_uint32(z.data(), 15);

         const int64_t S0 = X00 + X08 + X09 + X10 + X11 + X12 + 2 * (X13 + X14 + X15);
         const int64_t S1 = X01 + X09 + X10 + X11 + X12 + X13 + 2 * (X14 + X15);
         const int64_t S2 = X02 - (X08 + X09 + X13 + X14);
         const int64_t S3 = X03 + X08 + X11 + X12 + 2 * X13 + X14 + X15;
         const int64_t S4 = X04 + X09 + X12 + X13 + 2 * X14 + X15;
         const int64_t S5 = X05 + X10 + X13 + X14 + 2 * X15;
         const int64_t S6 = X06 + X11 + X14 + X15;
         const int64_t S7 = X07 + X08 + X09 + X10 + X11 + 2 * (X12 + X13 + X14 + X15) + X15;

         std::array<W, N> r = {};

         SolinasAccum sum(r);

         sum.accum(S0);
         sum.accum(S1);
         sum.accum(S2);
         sum.accum(S3);
         sum.accum(S4);
         sum.accum(S5);
         sum.accum(S6);
         sum.accum(S7);
         const auto S = sum.final_carry(0);

         const auto correction = sm2_mul_mod_256(S);
         W borrow = bigint_sub2(r.data(), N, correction.data(), N);

         bigint_cnd_add(borrow, r.data(), N, P.data(), N);

         return r;
      }

      constexpr static std::array<W, N> one() { return std::array<W, N>{1}; }

      constexpr static std::array<W, N> to_rep(const std::array<W, N>& x) { return x; }

      constexpr static std::array<W, N> wide_to_rep(const std::array<W, 2 * N>& x) { return redc(x); }

      constexpr static std::array<W, N> from_rep(const std::array<W, N>& z) { return z; }

   private:
      // Return (i*P) % 2**256
      //
      // Assumes i is small
      constexpr static std::array<W, N> sm2_mul_mod_256(W i) {
         static_assert(WordInfo<W>::bits == 32 || WordInfo<W>::bits == 64);

         // For small i, multiples of P have a simple structure so it's faster to
         // compute the value directly vs a (constant time) table lookup

         auto r = P;
         if constexpr(WordInfo<W>::bits == 32) {
            r[7] -= i;
            r[3] -= i;
            r[2] += i;
            r[0] -= i;
         } else {
            const uint64_t i32 = static_cast<uint64_t>(i) << 32;
            r[3] -= i32;
            r[1] -= i32;
            r[1] += i;
            r[0] -= i;
         }
         return r;
      }
};

// clang-format off

class Params final : public EllipticCurveParameters<
  "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF",
  "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC",
  "28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93",
  "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123",
  "32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7",
  "BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0"> {
};

// clang-format on

class Curve final : public EllipticCurve<Params, Sm2p256v1Rep> {
   public:
      // Return the square of the inverse of x
      static FieldElement fe_invert2(const FieldElement& x) {
         // Generated by https://github.com/mmcloughlin/addchain
         auto z = x.square();
         auto t0 = x * z;
         z = t0.square();
         z *= x;
         auto t1 = z;
         t1.square_n(3);
         t1 *= z;
         auto t2 = t1.square();
         z = t2 * x;
         t2.square_n(5);
         t1 *= t2;
         t2 = t1;
         t2.square_n(12);
         t1 *= t2;
         t1.square_n(7);
         z *= t1;
         t2 = z;
         t2.square_n(2);
         t1 = t2;
         t1.square_n(29);
         z *= t1;
         t1.square_n(2);
         t2 *= t1;
         t0 *= t2;
         t1.square_n(32);
         t1 *= t0;
         t1.square_n(64);
         t0 *= t1;
         t0.square_n(94);
         z *= t0;
         z.square_n(2);
         return z;
      }
};

}  // namespace sm2p256v1

}  // namespace

std::shared_ptr<const PrimeOrderCurve> PCurveInstance::sm2p256v1() {
   return PrimeOrderCurveImpl<sm2p256v1::Curve>::instance();
}

}  // namespace Botan::PCurve
