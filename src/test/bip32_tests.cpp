// Copyright (c) 2013-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>

#include "base58.h"
#include "key.h"
#include "uint256.h"
#include "util.h"
#include "utilstrencodings.h"
#include "test/test_pigeon.h"

#include <string>
#include <vector>

struct TestDerivation {
    std::string pub;
    std::string prv;
    unsigned int nChild;
};

struct TestVector {
    std::string strHexMaster;
    std::vector<TestDerivation> vDerive;

    TestVector(std::string strHexMasterIn) : strHexMaster(strHexMasterIn) {}

    TestVector& operator()(std::string pub, std::string prv, unsigned int nChild) {
        vDerive.push_back(TestDerivation());
        TestDerivation &der = vDerive.back();
        der.pub = pub;
        der.prv = prv;
        der.nChild = nChild;
        return *this;
    }
};

TestVector test1 =
  TestVector("000102030405060708090a0b0c0d0e0f")
    ("2oFskQ7gQyDcMmsjMkBa5TN82qHYDcBKHbdaPLVnYhziT2qm11mJRAjuvNLepsCkKDN5Udn5VRvEGweh1vaSjUaW7qm6c3wWT3dowu4S24L4tnCV",
     "2oFskQ7gQyDcMmsjMkBa5TN82qHYDcBKHbdaPLVnYhziT2qm11mJRAjuvNLeps8FHnep17ydYqRL3y76AjVoP51A1yaggnF6bjS7Q4oiWn6S7tc4",
     0x80000000)
    ("2oFskQ9wpxvups8rLtnhnf7zfiKeKvzsdyi4Q27DSg66a12xHP3bbayT1VV1EeUK1xfz1dzN4J9mhgHm6tyQsRcEVueZLFaJhxwiAQzZaMzn1ABX",
     "2oFskQ9wpxvups8rLtnhnf7zfiKeKvzsdyi4Q27DSg66a12xHP3bbayT1VV1EePbdCNBxq2XFCievMTw3TFq6ibSCmemCr7mjyPpHmpMNDCZ4eND",
     1)
    ("2oFskQC7xAiTiFqjQjEkEPNtYK4XyzNrjyjngxRtHGMsNwfkVcb6jJmsKT3uU4YmbNchMbhz75aDontFaVH9gtwQPbJPGYn9ZSaiS3j6MevRh4ej",
     "2oFskQC7xAiTiFqjQjEkEPNtYK4XyzNrjyjngxRtHGMsNwfkVcb6jJmsKT3uU4SngoHy1H7Gw1YFMSAtTbCMKqwEBYpFeMtjU39b9Scfjk7FnqzT",
     0x80000002)
    ("2oFskQEjEDFHZxiapbQYKdyHug4a8DXjGFHNgHy7xWhAY7Y7w2SHr7XhkMVTPcZxRpV8fxE5qgxCRAppYwcHyc2wvekY78g7hXXn7mR1Vk8NJqu9",
     "2oFskQEjEDFHZxiapbQYKdyHug4a8DXjGFHNgHy7xWhAY7Y7w2SHr7XhkMVTPcV1JmXUVnaJKc1CGraiMpQ65Rx9NfuhJiWJ2C5q63tQxNi81UGM",
     2)
    ("2oFskQGxd3gQX9CFoft23LEVzZZjSJM1zssgsjT1Fmeh1RygYPXvDBJQ7Z4vKHtwpEHHQRYD1pxgNDZifW2dyhLP8NM1N3TftMifwn6vLtm4uPaN",
     "2oFskQGxd3gQX9CFoft23LEVzZZjSJM1zssgsjT1Fmeh1RygYPXvDBJQ7Z4vKHoQmEBsvrNS2DvRqmQAXHazXdNgisJHM7kP7kAeoCrt7tEZqx66",
     1000000000);

TestVector test2 =
  TestVector("fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542")
    ("2oFskQ7gQyDcMmsjMjo5QZXCKHVFyrNTiHFqD25aGzwTH1DssJkKcLtgroEpRDJqHvcSSYLKHLatSLjCGeNAxbmRGyg6Yushut8JcDS78LDUuLon",
     "2oFskQ7gQyDcMmsjMjo5QZXCKHVFyrNTiHFqD25aGzwTH1DssJkKcLtgroEpRDC2LyAN6N2NyahnWFD8Mq2XV7RYiax1TEfAqsn28e4Lk91bQ4t3",
     0)
    ("2oFskQAxAEwJKFtZSG4fhhJaCMHfWDLCxzEKVaFKeAkUj5djpzimavA2vx5PfnmoBihSskeoTCotBjp3EoXbeUv7hKGkqBvSanUSzLvDnmTHJTJJ",
     "2oFskQAxAEwJKFtZSG4fhhJaCMHfWDLCxzEKVaFKeAkUj5djpzimavA2vx5PfnhJ9cECvmgcoUiCEgNYVBYAJfi3mXBArEqauGGBsmUFJWMAdyai",
     0xFFFFFFFF)
    ("2oFskQC7DVYKqdnjYRvst7kZqpdM4c5RsXiVBfQQB69PVjNWDT213Y3TFA9BGQA3JUS3yYAizrejo32TqUCTzFpbp3RDTj14e9Ci1DJVPcEPPmBB",
     "2oFskQC7DVYKqdnjYRvst7kZqpdM4c5RsXiVBfQQB69PVjNWDT213Y3TFA9BGQ3n8LUwQfuwhFjPsjP3bcmu5Q5eu6UcQfZpF2mv8ZFaqGi8SyZx",
     1)
    ("2oFskQEvBuZJmoS2jXYZ2BQiQnaHHwcJNduQSyqu2wD3q3Frf7qsCqYyx43ZeExESGJkG8MBYhHq2g9mfYzj1Aj2mJkgNKHGVMk9D95ALrf4re7B",
     "2oFskQEvBuZJmoS2jXYZ2BQiQnaHHwcJNduQSyqu2wD3q3Frf7qsCqYyx43ZeEqykNC96ysJQ12UG8viowUgESyBH6xRhoLM3sstX341ZkLBxkna",
     0xFFFFFFFE)
    ("2oFskQG6DpXF8QpzSpVncutXxA5yUrLmjgt2x1YcvTvkEt27uaCC8wDuxaZvjscMwTgGi5nqBVntjim6XCdtPrc9H27NpbBayv3Ed1JEvPdoEoXd",
     "2oFskQG6DpXF8QpzSpVncutXxA5yUrLmjgt2x1YcvTvkEt27uaCC8wDuxaZvjsYi8zcXPXSb6JrFFXJJPkbmbxv5mno6sdGtN25v9Ne25p9djBP4",
     2)
    ("2oFskQHTFmxTdw1JgZkyWdxfe8qMTFQJqwCx2fiEosi6jWoyVePXi6rqim8sjx1SDbFdQbr3QmiQ9cU7Wh4wtghqBLNEQEMR4rQXMgbS74MQpHvV",
     "2oFskQHTFmxTdw1JgZkyWdxfe8qMTFQJqwCx2fiEosi6jWoyVePXi6rqim8sjwxP99dcTZkSpbbt4yQ95o9f7hESzqmqbS8Ps5W3s51MdtnzfKYA",
     0);

TestVector test3 =
  TestVector("4b381541583be4423346c643850da4b320e46a87ae3d2a4e6da11eba819cd4acba45d239319ac14f863b8d5ab5a0d0c64d2e8a1e7d1457df2e5a3c51c73235be")
    ("2oFskQ7gQyDcMmsjMirXa5uh7VUX1j1np37zUjRLqQEUqGuxfGzYt12vpkW3whJNGkD25iR59AamSjQ285fosSUTiQ1kh37c8cgkT19cprmazyry",
     "2oFskQ7gQyDcMmsjMirXa5uh7VUX1j1np37zUjRLqQEUqGuxfGzYt12vpkW3whBkWvBbRSj3j9dUc9ds5Bg8oLrh9NRWhkwjV5DN6vn16nxU2ex6",
      0x80000000)
    ("2oFskQA3ciBSVTF6hvPcyK42cYjGgRNQCByQP8uanDUAgDDt2Y1gLaXhgpxqf12CnUZcpunZywYXBaMDci8kDNiPtyG2diDZ2AYFFVKyjXC5wvGX",
     "2oFskQA3ciBSVTF6hvPcyK42cYjGgRNQCByQP8uanDUAgDDt2Y1gLaXhgpxqezx6sG8vLtsh9AaGUoNVXqDmwv5Gu8BiLC5eXpNdv3nxkkJTRnt8",
      0);

void RunTest(const TestVector &test) {
    std::vector<unsigned char> seed = ParseHex(test.strHexMaster);
    CExtKey key;
    CExtPubKey pubkey;
    key.SetMaster(&seed[0], seed.size());
    pubkey = key.Neuter();
    for (const TestDerivation &derive : test.vDerive) {
        unsigned char data[74];
        key.Encode(data);
        pubkey.Encode(data);

        // Test private key
        CBitcoinExtKey b58key; b58key.SetKey(key);
        if(b58key.ToString() != derive.prv.c_str()){
            printf("ExpecteDPrivKey : %s\n",derive.prv.c_str());
            printf("PrivKey: %s\n", b58key.ToString().c_str());

        }

        BOOST_CHECK(b58key.ToString() == derive.prv);

        CBitcoinExtKey b58keyDecodeCheck(derive.prv);
        CExtKey checkKey = b58keyDecodeCheck.GetKey();
        assert(checkKey == key); //ensure a base58 decoded key also matches

        // Test public key
        CBitcoinExtPubKey b58pubkey; b58pubkey.SetKey(pubkey);
        if(b58pubkey.ToString() != derive.pub.c_str()){
            printf("expectedpubkey : %s\n",derive.pub.c_str());
            printf("Pubkey: %s\n", b58pubkey.ToString().c_str());
        }

        BOOST_CHECK(b58pubkey.ToString() == derive.pub);

        CBitcoinExtPubKey b58PubkeyDecodeCheck(derive.pub);
        CExtPubKey checkPubKey = b58PubkeyDecodeCheck.GetKey();
        assert(checkPubKey == pubkey); //ensure a base58 decoded pubkey also matches

        // Derive new keys
        CExtKey keyNew;
        BOOST_CHECK(key.Derive(keyNew, derive.nChild));
        CExtPubKey pubkeyNew = keyNew.Neuter();
        if (!(derive.nChild & 0x80000000)) {
            // Compare with public derivation
            CExtPubKey pubkeyNew2;
            BOOST_CHECK(pubkey.Derive(pubkeyNew2, derive.nChild));
            BOOST_CHECK(pubkeyNew == pubkeyNew2);
        }
        key = keyNew;
        pubkey = pubkeyNew;

        CDataStream ssPub(SER_DISK, CLIENT_VERSION);
        ssPub << pubkeyNew;
        BOOST_CHECK(ssPub.size() == 75);

        CDataStream ssPriv(SER_DISK, CLIENT_VERSION);
        ssPriv << keyNew;
        BOOST_CHECK(ssPriv.size() == 75);

        CExtPubKey pubCheck;
        CExtKey privCheck;
        ssPub >> pubCheck;
        ssPriv >> privCheck;

        BOOST_CHECK(pubCheck == pubkeyNew);
        BOOST_CHECK(privCheck == keyNew);
    }
}

BOOST_FIXTURE_TEST_SUITE(bip32_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(bip32_test1) {
    RunTest(test1);
}

BOOST_AUTO_TEST_CASE(bip32_test2) {
    RunTest(test2);
}

BOOST_AUTO_TEST_CASE(bip32_test3) {
    RunTest(test3);
}

BOOST_AUTO_TEST_SUITE_END()
