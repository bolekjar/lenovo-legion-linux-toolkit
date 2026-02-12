// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include <QtTest>

// add necessary includes here
#include <Core/Logger.h>
#include <Core/StackTrace.h>

#include <Singleton/CPP11ThreadModel.h>
#include <Singleton/StaticCreationPolicy.h>
#include <Singleton/SingletonHolder.h>

#include <ConnectX/ConnectXSignedNumber.h>

#include <BigInt/BigIntegerLibrary.h>

using LoggerHolder = bj::framework::SingletonHolder<bj::framework::Logger,bj::framework::StaticCreationPolicy,bj::framework::DefaultThreadModel>;

class BJLibs : public QObject
{
    Q_OBJECT

public:
    BJLibs();
    ~BJLibs();

private slots:
    void test_Logger();
    void test_StackTrace();
    void test_bigInteger();
    void test_connectX();
};

BJLibs::BJLibs()
{
    LoggerHolder::getInstance().init("test.log");
}

BJLibs::~BJLibs()
{}


void BJLibs::test_Logger()
{
    {
        QVERIFY_THROWS_EXCEPTION(bj::framework::Logger::LoggerException,bj::framework::Logger().write("TEST->THROW_EXCEPTION"));
    }

    {

        LOG_INFO("TEST->INFO",LoggerHolder);
        LOG_DEBUG("TEST->DEBUG",LoggerHolder);
        LOG_ERROR("TEST->ERROR",LoggerHolder);
        LOG_WARNING("TEST->WARNING",LoggerHolder);
    }
}

void BJLibs::test_StackTrace(){
    LOG_INFO(bj::framework::StackTrace::getFormatetStackTrace(),LoggerHolder);
}

void BJLibs::test_connectX()
{
    using SignedNumber  = bj::connectX::ConnectXSignedNumber<int64_t>;
    SignedNumber number = SignedNumber::getMinusInf();

    QVERIFY_THROWS_NO_EXCEPTION(number + -1);
    QVERIFY_THROWS_NO_EXCEPTION(number + 1);
    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,number + SignedNumber::getPlusInf());
    QVERIFY_THROWS_NO_EXCEPTION(number + SignedNumber::getMinusInf());
    QVERIFY_THROWS_NO_EXCEPTION(number + 0);


    number = SignedNumber::getPlusInf();

    QVERIFY_THROWS_NO_EXCEPTION(number + -1);
    QVERIFY_THROWS_NO_EXCEPTION(number + 1);
    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,number + SignedNumber::getMinusInf());
    QVERIFY_THROWS_NO_EXCEPTION(number + SignedNumber::getPlusInf());
    QVERIFY_THROWS_NO_EXCEPTION(number + 0);


    number = 10 + 10;

    QCOMPARE(number,20);

    number = SignedNumber::getPlusInf() + SignedNumber::getPlusInf();

    QCOMPARE(number,SignedNumber::getPlusInf());

    QCOMPARE(SignedNumber::getPlusInf()  > SignedNumber::getMinusInf(),true);
    QCOMPARE(SignedNumber::getMinusInf() < SignedNumber::getMin(),true);
    QCOMPARE(SignedNumber::getPlusInf()  > SignedNumber::getMax(),true);
    QCOMPARE(SignedNumber::getPlusInf()  >= SignedNumber::getPlusInf(),true);



    number = SignedNumber::getMax();

    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,number - -1);
    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,number - -10);

    number = SignedNumber::getMin();

    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,number - 1);
    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,number - 10);

    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,SignedNumber::getMinusInf() - SignedNumber::getMinusInf());


    QCOMPARE(SignedNumber( 100)* 100,10000);
    QCOMPARE(SignedNumber::getPlusInf() * 1000,SignedNumber::getPlusInf());
    QCOMPARE(SignedNumber::getMinusInf() * 1000,SignedNumber::getMinusInf());
    QCOMPARE(SignedNumber::getMinusInf() * -1000,SignedNumber::getPlusInf());
    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,SignedNumber::getPlusInf() * 0);
    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,SignedNumber::getMinusInf() * 0);
    QCOMPARE(SignedNumber::getMax() * 0,0);
    QCOMPARE(SignedNumber::getMin() * 0,0);


    QCOMPARE(SignedNumber::getMin() / SignedNumber::getMin(),1);
    QCOMPARE(SignedNumber(100) /  2,50);
    QVERIFY_THROWS_EXCEPTION( SignedNumber::exception_T, SignedNumber::getPlusInf()  / SignedNumber::getPlusInf());
    QVERIFY_THROWS_EXCEPTION( SignedNumber::exception_T, SignedNumber::getMinusInf() / SignedNumber::getPlusInf());
    QVERIFY_THROWS_EXCEPTION( SignedNumber::exception_T,SignedNumber::getPlusInf()   / 0);
    QCOMPARE(SignedNumber(100) /  SignedNumber::getMinusInf(),0);


    QCOMPARE(SignedNumber::getMin() % SignedNumber::getMin(),0);
    QVERIFY_THROWS_EXCEPTION( SignedNumber::exception_T, SignedNumber::getMinusInf() % SignedNumber::getPlusInf());


    QVERIFY_THROWS_EXCEPTION( SignedNumber::exception_T, SignedNumber::getMin()--);
    QVERIFY_THROWS_EXCEPTION( SignedNumber::exception_T, --SignedNumber::getMin());

    QVERIFY_THROWS_EXCEPTION( SignedNumber::exception_T, SignedNumber::getMax()++);
    QVERIFY_THROWS_EXCEPTION( SignedNumber::exception_T, ++SignedNumber::getMax());


    SignedNumber tmp = SignedNumber::getMax();
    tmp--;
    QCOMPARE(tmp,SignedNumber::getMax() - 1 );

    tmp = SignedNumber::getMin();
    tmp++;
    QCOMPARE(tmp,SignedNumber::getMin() + 1 );


    tmp = SignedNumber::getMax();

    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,tmp += 1);

    tmp = SignedNumber::getMin();

    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,tmp -= 1);

    tmp = SignedNumber::getMin() / 1000;

    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,tmp *= 200000);

    tmp = SignedNumber::getMax() / 1000;

    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,tmp *= 200000);


    tmp = 1000;

    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,tmp %= SignedNumber::getPlusInf());
    QVERIFY_THROWS_EXCEPTION(SignedNumber::exception_T,tmp %= SignedNumber::getMinusInf());

    tmp = 1000;

    tmp %= 5000;

    QCOMPARE(tmp,1000);
}

using bj::framework::BigUnsigned;
using bj::framework::BigInteger;
using bj::framework::BigUnsignedInABase;
using bj::framework::stringToBigInteger;
using bj::framework::stringToBigUnsigned;

int16_t pathologicalShort = ~((uint16_t)(~0) >> 1);
int32_t pathologicalInt = ~((uint32_t)(~0) >> 1);
int64_t pathologicalLong = ~((uint64_t)(~0) >> 1);

void BJLibs::test_bigInteger() {

    BigUnsigned z(0), one(1), ten(10);
    QCOMPARE(z,0); //0
    QCOMPARE(one,1); //1
    QCOMPARE(ten,10); //10

    // === Default constructors ===
    QCOMPARE(BigUnsigned(),0); //0
    QCOMPARE(BigInteger(),0); //0



    // === Block-array constructors ===
    BigUnsigned::Blk myBlocks[3];
    myBlocks[0] = 1;
    myBlocks[1] = 1;
    myBlocks[2] = 0;
    BigUnsigned bu(myBlocks, 3);


    QCOMPARE(bu,stringToBigUnsigned("18446744073709551617"));
    QCOMPARE(BigInteger(myBlocks, 3),stringToBigInteger("18446744073709551617"));
    QCOMPARE(BigInteger(bu         ),stringToBigInteger("18446744073709551617"));


    // For nonzero magnitude, reject zero and invalid signs.
    QCOMPARE(BigInteger(myBlocks, 3, BigInteger::positive),stringToBigInteger("18446744073709551617")); //18446744073709551617
    QCOMPARE(BigInteger(myBlocks, 3, BigInteger::negative),stringToBigInteger("-18446744073709551617")); //-18446744073709551617
    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,BigInteger(myBlocks, 3, BigInteger::zero)); //error
    QCOMPARE(BigInteger(bu,          BigInteger::positive),stringToBigInteger("18446744073709551617")); //18446744073709551617
    QCOMPARE(BigInteger(bu,          BigInteger::negative),stringToBigInteger("-18446744073709551617")); //-18446744073709551617
    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,BigInteger(bu,          BigInteger::zero    )); //erro

    // For zero magnitude, force the sign to zero without error.
    BigUnsigned::Blk myZeroBlocks[1];
    myZeroBlocks[0] = 0;
    QCOMPARE(BigInteger(myZeroBlocks, 1, BigInteger::positive),0); //0
    QCOMPARE(BigInteger(myZeroBlocks, 1, BigInteger::negative),0); //0
    QCOMPARE(BigInteger(myZeroBlocks, 1, BigInteger::zero    ),0); //0

    // === BigUnsigned conversion limits ===
    QCOMPARE(BigUnsigned(0).toUnsignedInt64(),0U); //0
    QCOMPARE(BigUnsigned(18446744073709551615U).toUnsignedInt64(),18446744073709551615U); //18446744073709551615U
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,stringToBigUnsigned("18446744073709551616").toUnsignedInt64()); //error

    QCOMPARE(BigUnsigned(0).toInt64(),0L); //0
    QCOMPARE(BigUnsigned(9223372036854775807).toInt64(),9223372036854775807L); //9223372036854775807
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned(9223372036854775808U).toInt64()); //error

    // int is the 32 bits and long is 64 bits on a 64-bit system
    QCOMPARE(BigUnsigned(0).toUnsignedInt32(),0U); //0
    QCOMPARE(BigUnsigned(4294967295U).toUnsignedInt32(),4294967295U); //4294967295
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,stringToBigUnsigned("4294967296").toUnsignedInt32()); //error

    QCOMPARE(BigUnsigned(0).toInt32(),0); //0
    QCOMPARE(BigUnsigned(2147483647).toInt32(),2147483647); //2147483647
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned(2147483648U).toInt32()); //error

    QCOMPARE(BigUnsigned(0).toUnsignedInt16(),0); //0
    QCOMPARE(BigUnsigned(65535).toUnsignedInt16(),65535); //65535
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned(65536).toUnsignedInt16()); //error

    QCOMPARE(BigUnsigned(0).toInt16(),0); //0
    QCOMPARE(BigUnsigned(32767).toInt16(),32767); //32767
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned(32768).toInt16()); //error


    // === BigInteger conversion limits ===

    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,BigInteger(-1).toUnsignedInt64()); //error
    QCOMPARE(BigInteger(0).toUnsignedInt64(),0U); //0
    QCOMPARE(BigInteger(18446744073709551615U).toUnsignedInt64(),18446744073709551615U); //4294967295
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,stringToBigInteger("18446744073709551616").toUnsignedInt64()); //error


    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,stringToBigInteger("-9223372036854775809").toInt64()); //error
    QCOMPARE(stringToBigInteger("-9223372036854775807").toInt64(),-9223372036854775807); //-2147483648
    QCOMPARE(BigInteger(-9223372036854775807).toInt64(),-9223372036854775807); //-2147483647
    QCOMPARE(BigInteger(0).toInt64(),0); //0
    QCOMPARE(BigInteger(9223372036854775807).toInt64(),9223372036854775807); //2147483647
    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,BigInteger(9223372036854775808U).toInt64()); //error

    // int is the 32 bits and long is 64 bits on a 64-bit system
    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,BigInteger(-1).toUnsignedInt32()); //error
    QCOMPARE(BigInteger(0).toUnsignedInt32(),0U); //0
    QCOMPARE(BigInteger(4294967295U).toUnsignedInt32(),4294967295U); //4294967295
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,stringToBigInteger("4294967296").toUnsignedInt32()); //error

    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,stringToBigInteger("-2147483649").toInt32()); //error
    QCOMPARE(stringToBigInteger("-2147483648").toInt32(),-2147483648); //-2147483648
    QCOMPARE(BigInteger(-2147483647).toInt32(),-2147483647); //-2147483647
    QCOMPARE(BigInteger(0).toInt32(),0); //0
    QCOMPARE(BigInteger(2147483647).toInt32(),2147483647); //2147483647
    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,BigInteger(2147483648U).toInt32()); //error

    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,BigInteger(-1).toUnsignedInt16()); //error
    QCOMPARE(BigInteger(0).toUnsignedInt16(),0); //0
    QCOMPARE(BigInteger(65535).toUnsignedInt16(),65535); //65535
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigInteger(65536).toUnsignedInt16()); //error

    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,BigInteger(-32769).toInt16()); //error
    QCOMPARE(BigInteger(-32768).toInt16(),-32768); //-32768
    QCOMPARE(BigInteger(-32767).toInt16(),-32767); //-32767
    QCOMPARE(BigInteger(0).toInt16(),0); //0
    QCOMPARE(BigInteger(32767).toInt16(),32767); //32767
    QVERIFY_THROWS_EXCEPTION(BigInteger::exception_T,BigInteger(32768).toInt16()); //error

    // === Negative BigUnsigneds ===


    // ...during construction
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned(int16_t(-1))); //error
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned varname(pathologicalShort)); //error
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned(int32_t(-1))); //error
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned varname(pathologicalInt)); //error
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned(int64_t(-1))); //error
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned varname(pathologicalLong)); //error

    // ...during subtraction
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned(5) - BigUnsigned(6)); //error
    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,stringToBigUnsigned("314159265358979323") - stringToBigUnsigned("314159265358979324")); //error
    QCOMPARE(BigUnsigned(5) - BigUnsigned(5),0); //0
    QCOMPARE(stringToBigUnsigned("314159265358979323") - stringToBigUnsigned("314159265358979323"),0); //0
    QCOMPARE(stringToBigUnsigned("4294967296") - BigUnsigned(1),stringToBigUnsigned("4294967295")); //4294967295


    // === BigUnsigned addition ===

    QCOMPARE(BigUnsigned(0) + 0,0); //0
    QCOMPARE(BigUnsigned(0) + 1,1); //1
    // Ordinary carry
    QCOMPARE(stringToBigUnsigned("8589934591" /* 2^33 - 1*/)
            + stringToBigUnsigned("4294967298" /* 2^32 + 2 */),stringToBigUnsigned("12884901889")); //12884901889
    // Creation of a new block
    QCOMPARE(BigUnsigned(0xFFFFFFFFU) + 1,stringToBigUnsigned("4294967296")); //4294967296

    // === BigUnsigned subtraction ===

    QCOMPARE(BigUnsigned(1) - 0,1); //1
    QCOMPARE(BigUnsigned(1) - 1,0); //0
    QCOMPARE(BigUnsigned(2) - 1,1); //1
    // Ordinary borrow
    QCOMPARE(stringToBigUnsigned("12884901889")
            - stringToBigUnsigned("4294967298"),stringToBigUnsigned("8589934591")); //8589934591
    // Borrow that removes a block
    QCOMPARE(stringToBigUnsigned("4294967296") - 1,stringToBigUnsigned("4294967295")); //4294967295

    // === BigUnsigned multiplication and division ===

    BigUnsigned a = BigUnsigned(314159265) * 358979323;
    QCOMPARE(a,stringToBigUnsigned("112776680263877595")); //112776680263877595
    QCOMPARE(a / 123,stringToBigUnsigned("916883579381118")); //916883579381118
    QCOMPARE(a % 123,stringToBigUnsigned("81")); //81

    QVERIFY_THROWS_EXCEPTION(BigUnsigned::exception_T,BigUnsigned(5) / 0); //error

    // === Block accessors ===

    BigUnsigned b;
    QCOMPARE(b,0); //0
    QCOMPARE(b.getBlock(0),0U); //0
    b.setBlock(1, 314);

    // Did b grow properly?  And did we zero intermediate blocks?
    QCOMPARE(b,stringToBigUnsigned("5792277639144799207424")); //5792277639144799207424
    QCOMPARE(b.getLength(),2U); //2
    QCOMPARE(b.getBlock(0),0U); //0
    QCOMPARE(b.getBlock(1),314U); //314
    // Did b shrink properly?
    b.setBlock(1, 0);
    QCOMPARE(b,0); //0

    BigUnsigned bb(314);
    bb.setBlock(1, 159);
    // Make sure we used allocateAndCopy, not allocate
    QCOMPARE(bb.getBlock(0),314U); //314
    QCOMPARE(bb.getBlock(1),159U); //159
    // Blocks beyond the number should be zero regardless of whether they are
    // within the capacity.
    bb.add(1, 2);
    QCOMPARE(bb.getBlock(0),3U); //3
    QCOMPARE(bb.getBlock(1),0U); //0
    QCOMPARE(bb.getBlock(2),0U); //0
    QCOMPARE(bb.getBlock(314159),0U); //0

    // === Bit accessors ===

    QCOMPARE(BigUnsigned(0).bitLength(),0U); //0
    QCOMPARE(BigUnsigned(1).bitLength(),1U); //1
    QCOMPARE(BigUnsigned(4095).bitLength(),12U); //12
    QCOMPARE(BigUnsigned(4096).bitLength(),13U); //13
    // 5 billion is between 2^32 (about 4 billion) and 2^33 (about 8 billion).
    QCOMPARE(stringToBigUnsigned("5000000000").bitLength(),33U); //33

    // 25 is binary 11001.
    BigUnsigned bbb(25);
    QCOMPARE(bbb.getBit(4),1); //1
    QCOMPARE(bbb.getBit(3),1); //1
    QCOMPARE(bbb.getBit(2),0); //0
    QCOMPARE(bbb.getBit(1),0); //0
    QCOMPARE(bbb.getBit(0),1); //1
    QCOMPARE(bbb.bitLength(),5U); //5
    // Effectively add 2^32.
    bbb.setBit(32, true);
    QCOMPARE(bbb,stringToBigUnsigned("4294967321")); //4294967321
    bbb.setBit(31, true);
    bbb.setBit(32, false);
    QCOMPARE(bbb,stringToBigUnsigned("2147483673")); //2147483673

    // === Combining BigUnsigned, BigInteger, and primitive integers ===

    BigUnsigned p1 = BigUnsigned(3) * 5;
    QCOMPARE(p1,15); //15
    /* In this case, we would like g++ to implicitly promote the BigUnsigned to a
     * BigInteger, but it seems to prefer converting the -5 to a BigUnsigned, which
     * causes an error.  If I take out constructors for BigUnsigned from signed
     * primitive integers, the BigUnsigned(3) becomes ambiguous, and if I take out
     * all the constructors but BigUnsigned(unsigned long), g++ uses that
     * constructor and gets a wrong (positive) answer.  Thus, I think we'll just
     * have to live with this cast. */
    BigInteger p2 = BigInteger(BigUnsigned(3)) * -5;
    QCOMPARE(p2,-15); //-15

    // === Test some previous bugs ===

    {
        /* Test that BigInteger division sets the sign to zero.
         * Bug reported by David Allen. */
        BigInteger num(3), denom(5), quotient;
        num.divideWithRemainder(denom, quotient);
        QCOMPARE(quotient,0);
        num = 5;
        num.divideWithRemainder(denom, quotient);
        QCOMPARE(num,0);
    }

    {
        /* Test that BigInteger subtraction sets the sign properly.
         * Bug reported by Samuel Larkin. */
        BigInteger zero(0), three(3), ans;
        ans = zero - three;
        QCOMPARE(ans.getSign(),-1); //-1
    }

    {
        /* Test that BigInteger multiplication shifts bits properly on systems
         * where long is bigger than int.  (Obviously, this would only catch the
         * bug when run on such a system.)
         * Bug reported by Mohand Mezmaz. */
        BigInteger f=4; f*=3;
        QCOMPARE(f,12); //12
    }

    {
        /* Test that bitwise XOR allocates the larger length.
         * Bug reported by Sriram Sankararaman. */
        BigUnsigned a(0), b(3), ans;
        ans = a ^ b;
        QCOMPARE(ans,3); //3
    }

    {
        /* Test that an aliased multiplication works.
         * Bug reported by Boris Dessy. */
        BigInteger num(5);
        num *= num;
        QCOMPARE(num,25); //25
    }

    {
        /* Test that BigUnsignedInABase(std::string) constructor rejects digits
         * too big for the specified base.
         * Bug reported by Niakam Kazemi. */
        QVERIFY_THROWS_EXCEPTION(BigUnsignedInABase::exception_T,BigUnsignedInABase("f", 10)); //error
    }

}

QTEST_APPLESS_MAIN(BJLibs)

#include "tst_BJLibs.moc"
