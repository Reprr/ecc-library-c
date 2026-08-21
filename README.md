# ECC Library

A from-scratch implementation of elliptic curve cryptography primitives in C99,
following **SEC 1 v2.0**. Targets the **secp256k1** curve. Zero external dependencies.

> **Warning.** Educational project. Not audited, not constant-time, no side-channel
> protection. Do not use in production.

## Implemented

**Arithmetic**
- 256-bit integer arithmetic (add, sub, mul, modular reduction, shifts, comparisons)
- Modular inversion via Fermat's little theorem
- Affine and projective point operations (addition, doubling, scalar multiplication)

**SEC 1 primitives**
- Key pair generation (§3.2.1) and public key validation (§3.2.2)
- ECDH primitive (§3.3.1)
- Point serialization: uncompressed / compressed (§2.3.3), private keys (§2.3.7)

**Hashing and key derivation**
- SHA-256 (FIPS 180-4), streaming `init / update / finalize` API
- HMAC-SHA-256 (RFC 2104)
- ANSI X9.63 KDF (§3.6.1)

**In progress**
- ECDSA (§4.1), RFC 6979 deterministic nonces
- ECIES (§5.1)

## Building

```sh
make                # static library + tests + example
make shared         # shared library (libecc.so)
make run-tests      # run the full test suite
make ASAN=1 run-tests   # same under AddressSanitizer / UBSan
```

Linking from an external project:

```sh
gcc app.c -Iinclude -Iinclude/ecc -Lbuild/lib -lecc -lm
```

## Example

`examples/ecdh_example.c` performs a full ECDH exchange: key generation,
compressed serialization, deserialization on the peer side, shared secret
derivation on both sides, and session key derivation.

```sh
$ ./build/bin/ecdh_example
Alice pub   : 035e703479efa0e95155ef2eef279a06e69eee412ede348ea1c41bc0c57d7d570a
Bob pub     : 03ec69935c7294ca493f4d865ea555e67ed03c2aa3b8736d6558268ff6a8ce578a
Shared Z    : bdc945651ac6d9433295b996f61aae50fc6a38fc27101de143ed3d0d02f29415
Session key : dbd5ea51f3c9a2675929401e0aa8614aad24d6dd8e1bdade69f954d49bce2078
ECDH OK
```

## Testing

Unity-based suite covering big-integer edge cases, projective point arithmetic,
SEC 1 serialization round-trips and malformed-input rejection, ECDH symmetry,
and SHA-256 FIPS 180-4 vectors (including padding boundary lengths).

## Layout

```
include/ecc/   public headers
src/           implementation
test/          Unity test suite
examples/      usage examples
```

## Security considerations

- Branching and timing depend on secret data (no constant-time guarantees)
- No side-channel mitigations
- No independent audit

For production cryptography use libsodium, OpenSSL, or BoringSSL.

## References

- SEC 1 v2.0 — Elliptic Curve Cryptography, Certicom
- FIPS 180-4 — Secure Hash Standard (SHA-2)
- RFC 2104 — HMAC: Keyed-Hashing for Message Authentication
- RFC 6979 — Deterministic Usage of DSA and ECDSA
