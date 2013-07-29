#ifndef CPPUNIT_SAMPLE
#define CPPUNIT_SAMPLE

#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/Asserter.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/extensions/HelperMacros.h>


class Complex {
    friend bool operator ==(const Complex& a, const Complex& b);

    double real, imaginary;
public:
    Complex(double r, double i = 0)
    : real(r)
    , imaginary(i) {
    }
};

bool operator ==(const Complex &a, const Complex &b) {
    return a.real == b.real && a.imaginary == b.imaginary;
};

class ComplexNumberTest : public CppUnit::TestFixture {
private:
    Complex *m_10_1, *m_1_1, *m_11_2;

    CPPUNIT_TEST_SUITE( ComplexNumberTest );
    CPPUNIT_TEST( testEquality );
    CPPUNIT_TEST( testAddition );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
        m_10_1 = new Complex(10, 1);
        m_1_1 = new Complex(1, 1);
        m_11_2 = new Complex(11, 2);
    }

    void tearDown() {
        delete m_10_1;
        delete m_1_1;
        delete m_11_2;
    }

    void testEquality() {
        std::cout << m_10_1 << std::endl;
        CPPUNIT_ASSERT( *m_10_1 == *m_10_1 );
        CPPUNIT_ASSERT( false );
        CPPUNIT_ASSERT( !(*m_10_1 == *m_11_2) );
    }

    void testAddition() {
        std::cout << m_10_1 << std::endl;
    }

};

int initCppUnit() {
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(ComplexNumberTest::suite());
    runner.run();
}
#endif