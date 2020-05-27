import unittest


class Test01(unittest.TestCase):

    def test_conversions(self):
        from fractions import Fraction as F
        import pybind11_test_01 as p

        self.assertTrue(p.test_int1_conversion(0) == 0)
        self.assertTrue(p.test_int1_conversion(-1) == -1)
        self.assertTrue(p.test_int1_conversion(1) == 1)
        self.assertTrue(p.test_int1_conversion(
            123213123211233232321312321321) == 123213123211233232321312321321)
        self.assertTrue(p.test_int1_conversion(-123213123211233232321312321321)
                        == -123213123211233232321312321321)

        self.assertTrue(p.test_int2_conversion(0) == 0)
        self.assertTrue(p.test_int2_conversion(-1) == -1)
        self.assertTrue(p.test_int2_conversion(1) == 1)
        self.assertTrue(p.test_int2_conversion(
            123213123211233232321312321321) == 123213123211233232321312321321)
        self.assertTrue(p.test_int2_conversion(-123213123211233232321312321321)
                        == -123213123211233232321312321321)

        self.assertTrue(p.test_rat1_conversion(F(0)) == 0)
        self.assertTrue(p.test_rat1_conversion(F(-1)) == -1)
        self.assertTrue(p.test_rat1_conversion(F(1)) == 1)
        self.assertTrue(p.test_rat1_conversion(F(3, 4)) == F(3, 4))
        self.assertTrue(p.test_rat1_conversion(F(3, -4)) == F(-6, 8))
        self.assertTrue(p.test_rat1_conversion(
            F(123213123211233232321312321321, -39032138128182331123)) == F(123213123211233232321312321321, -39032138128182331123))

        self.run_quadmath_conversions()
        self.run_mpfr_conversions()
        self.run_mpc_conversions()

    def run_quadmath_conversions(self):
        import pybind11_test_01 as p

        if not p.has_quadmath():
            return

        try:
            from mpmath import mpf, mpc, mp, workprec, isnan, isinf
        except ImportError:
            return

        # Default precision is 53, so the conversion will fail.
        self.assertRaises(TypeError, lambda: p.test_real128_conversion(mpf()))
        self.assertRaises(
            TypeError, lambda: p.test_complex128_conversion(mpc()))
        self.assertRaises(
            TypeError, lambda: p.test_real128_conversion(mpf("inf")))
        self.assertRaises(
            TypeError, lambda: p.test_complex128_conversion(mpc("inf")))
        self.assertRaises(
            TypeError, lambda: p.test_real128_conversion(mpf("nan")))
        self.assertRaises(
            TypeError, lambda: p.test_complex128_conversion(mpc("nan")))

        # Set precision to quadruple.
        mp.prec = 113
        self.assertTrue(p.test_real128_conversion(mpf()) == 0)
        self.assertTrue(p.test_complex128_conversion(mpc()) == 0)
        self.assertTrue(p.test_real128_conversion(mpf(1)) == 1)
        self.assertTrue(p.test_complex128_conversion(mpc(1)) == 1)
        self.assertTrue(p.test_real128_conversion(mpf(-1)) == -1)
        self.assertTrue(p.test_complex128_conversion(mpc(-1)) == -1)
        self.assertTrue(p.test_real128_conversion(mpf("inf")) == mpf("inf"))
        self.assertTrue(p.test_complex128_conversion(mpc("inf")) == mpc("inf"))
        self.assertTrue(p.test_complex128_conversion(
            mpc("inf", "inf")) == mpc("inf", "inf"))
        self.assertTrue(p.test_real128_conversion(-mpf("inf")) == -mpf("inf"))
        self.assertTrue(
            p.test_complex128_conversion(-mpc("inf")) == -mpc("inf"))
        self.assertTrue(isnan(p.test_real128_conversion(mpf("nan"))))
        self.assertTrue(p.test_real128_conversion(mpf(
            "1.32321321001020301293838201938121203")) == mpf("1.32321321001020301293838201938121203"))
        self.assertTrue(p.test_complex128_conversion(mpc(
            "1.32321321001020301293838201938121203")) == mpc("1.32321321001020301293838201938121203"))
        self.assertTrue(p.test_complex128_conversion(mpc(
            "1.32321321001020301293838201938121203", "4.48482020209340933923902320203402020")) == mpc("1.32321321001020301293838201938121203", "4.48482020209340933923902320203402020"))
        self.assertTrue(p.test_real128_conversion(-mpf(
            "1.32321321001020301293838201938121203")) == -mpf("1.32321321001020301293838201938121203"))
        self.assertTrue(p.test_complex128_conversion(-mpc(
            "1.32321321001020301293838201938121203")) == -mpc("1.32321321001020301293838201938121203"))
        self.assertTrue(p.test_complex128_conversion(-mpc(
            "1.32321321001020301293838201938121203", "4.48482020209340933923902320203402020")) == -mpc("1.32321321001020301293838201938121203", "4.48482020209340933923902320203402020"))
        self.assertTrue(p.test_real128_conversion(
            mpf(2)**-16382) == mpf(2)**-16382)
        self.assertTrue(p.test_complex128_conversion(
            mpc(2)**-16382) == mpc(2)**-16382)
        self.assertTrue(p.test_real128_conversion(
            -(mpf(2)**-16382)) == -(mpf(2)**-16382))
        self.assertTrue(p.test_complex128_conversion(
            -(mpc(2)**-16382)) == -(mpc(2)**-16382))
        # Try subnormal numbers behaviour.
        self.assertTrue(mpf(2)**-16495 != 0)
        self.assertTrue(p.test_real128_conversion(
            mpf(2)**-16495) == mpf(0))
        self.assertTrue(p.test_complex128_conversion(
            mpc(2)**-16495) == mpc(0))

        # Try the workprec construct.
        with workprec(2000):
            self.assertRaises(
                TypeError, lambda: p.test_real128_conversion(mpf()))
            self.assertRaises(
                TypeError, lambda: p.test_complex128_conversion(mpc()))
            self.assertRaises(
                TypeError, lambda: p.test_real128_conversion(mpf("inf")))
            self.assertRaises(
                TypeError, lambda: p.test_complex128_conversion(mpc("inf")))
            self.assertRaises(
                TypeError, lambda: p.test_real128_conversion(mpf("nan")))
            self.assertRaises(
                TypeError, lambda: p.test_complex128_conversion(mpc("nan")))

        # Test that the real128/complex128 overloads are picked before the real one,
        # if instantiated before.
        self.assertTrue(p.test_overload(mpf(2)**-16495) == 0)
        self.assertTrue(p.test_overload(mpc(2)**-16495) == 0)
        if not p.has_mpfr():
            with workprec(2000):
                self.assertTrue(p.test_overload(mpf(2)**-16495) != 0)
            with workprec(2000):
                self.assertTrue(p.test_overload(mpc(2)**-16495) != 0)

        # Test we don't end up to inf if the significand has more than 113 bits.
        mp.prec = 40000
        foo = mpf("1.1")
        with workprec(113):
            self.assertFalse(isinf(p.test_real128_conversion(foo)))
        foo = mpc("1.1", "1.2")
        with workprec(113):
            self.assertFalse(isinf(p.test_complex128_conversion(foo)))

        # Restore prec on exit.
        mp.prec = 53

    def run_mpfr_conversions(self):
        import pybind11_test_01 as p

        if not p.has_mpfr():
            return

        try:
            from mpmath import mpf, mp, workprec, isnan
        except ImportError:
            return

        self.assertTrue(p.test_real_conversion(mpf()) == 0)
        self.assertTrue(p.test_real_conversion(mpf(1)) == 1)
        self.assertTrue(p.test_real_conversion(mpf(-1)) == -1)
        self.assertTrue(p.test_real_conversion(mpf("inf")) == mpf("inf"))
        self.assertTrue(p.test_real_conversion(-mpf("inf")) == -mpf("inf"))
        self.assertTrue(isnan(p.test_real_conversion(mpf("nan"))))
        self.assertTrue(p.test_real_conversion(mpf(
            "1.323213210010203")) == mpf("1.323213210010203"))
        self.assertTrue(p.test_real_conversion(-mpf(
            "1.323213210010203")) == -mpf("1.323213210010203"))

        # Test with different precision.
        # Less precision in output from C++ is ok.
        self.assertTrue(p.test_real_conversion(mpf(-1), 30) == -1)
        self.assertTrue(p.test_real_conversion(mpf("inf"), 30) == mpf("inf"))
        self.assertTrue(isnan(p.test_real_conversion(mpf("nan"), 30)))
        # More precision in output from C++ will be lossy, so an error
        # is raised instead.
        self.assertRaises(
            ValueError, lambda: p.test_real_conversion(mpf(-1), 300))
        self.assertRaises(
            ValueError, lambda: p.test_real_conversion(mpf("inf"), 300))
        self.assertRaises(
            ValueError, lambda: p.test_real_conversion(mpf("nan"), 300))

        # Try the workprec construct.
        with workprec(100):
            self.assertTrue(p.test_real_conversion(mpf("1.1")) == mpf("1.1"))
            self.assertTrue(p.test_real_conversion(mpf("-1.1")) == mpf("-1.1"))

    def run_mpc_conversions(self):
        import pybind11_test_01 as p

        if not p.has_mpc():
            return

        try:
            from mpmath import mpc, mp, workprec, isnan
        except ImportError:
            return

        self.assertTrue(p.test_complex_conversion(mpc()) == 0)
        self.assertTrue(p.test_complex_conversion(mpc(1)) == 1)
        self.assertTrue(p.test_complex_conversion(mpc(1, 1)) == mpc(1, 1))
        self.assertTrue(p.test_complex_conversion(mpc(-1, -1)) == mpc(-1, -1))
        self.assertTrue(p.test_complex_conversion(
            mpc("inf", "inf")) == mpc("inf", "inf"))
        self.assertTrue(
            p.test_complex_conversion(-mpc("inf", "inf")) == -mpc("inf", "inf"))
        self.assertTrue(
            isnan(p.test_complex_conversion(mpc("nan", "nan")).real))
        self.assertTrue(
            isnan(p.test_complex_conversion(mpc("nan", "nan")).imag))
        self.assertTrue(p.test_complex_conversion(mpc(
            "1.323213210010203", "2.33343456633")) == mpc("1.323213210010203", "2.33343456633"))
        self.assertTrue(p.test_complex_conversion(-mpc(
            "1.323213210010203", "2.33343456633")) == -mpc("1.323213210010203", "2.33343456633"))

        # Test with different precision.
        # Less precision in output from C++ is ok.
        self.assertTrue(p.test_complex_conversion(mpc(-1), 30) == -1)
        self.assertTrue(p.test_complex_conversion(
            mpc("inf"), 30) == mpc("inf"))
        self.assertTrue(isnan(p.test_complex_conversion(mpc("nan"), 30).real))
        # More precision in output from C++ will be lossy, so an error
        # is raised instead.
        self.assertRaises(
            ValueError, lambda: p.test_complex_conversion(mpc(-1), 300))
        self.assertRaises(
            ValueError, lambda: p.test_complex_conversion(mpc("inf"), 300))
        self.assertRaises(
            ValueError, lambda: p.test_complex_conversion(mpc("nan"), 300))

        # Try the workprec construct.
        with workprec(100):
            self.assertTrue(p.test_complex_conversion(
                mpc("1.1", "2.3")) == mpc("1.1", "2.3"))
            self.assertTrue(p.test_complex_conversion(
                mpc("-1.1", "-2.3")) == mpc("-1.1", "-2.3"))

    def test_stl(self):
        from fractions import Fraction as F
        import pybind11_test_01 as p

        self.assertTrue(p.test_vector_conversion([]) == [])
        self.assertTrue(p.test_vector_conversion([1, -2, 3]) == [1, -2, 3])
        self.assertTrue(p.test_vector_conversion(
            [F(1), F(-2, 7), F(3.5)]) == [F(1), F(-2, 7), F(3.5)])
        self.assertRaises(
            TypeError, lambda: p.test_vector_conversion([1, -2, 3.]))
        self.assertRaises(
            TypeError, lambda: p.test_vector_conversion([1, -2, F(3)]))

        self.assertTrue(p.test_unordered_map_conversion({}) == {})
        self.assertTrue(p.test_unordered_map_conversion(
            {'a': 5, 'b': 6}) == {'a': 5, 'b': 6})
        self.assertTrue(p.test_unordered_map_conversion(
            {'a': F(5), 'b': F(6)}) == {'a': F(5), 'b': F(6)})
        self.assertRaises(TypeError, lambda: p.test_unordered_map_conversion(
            {'a': 5, 'b': 6.}))
        self.assertRaises(TypeError, lambda: p.test_unordered_map_conversion(
            {'a': F(5), 'b': 6}))

        self.run_quadmath_stl()
        self.run_mpfr_stl()
        self.run_mpc_stl()

    def run_quadmath_stl(self):
        import pybind11_test_01 as p

        if not p.has_quadmath():
            return

        try:
            from mpmath import mpf, mpc, mp, workprec, isnan
        except ImportError:
            return

        if not p.has_mpfr():
            self.assertRaises(TypeError, lambda: p.test_vector_conversion(
                [mpf(1), mpf(-2), mpf(3)]))
            self.assertRaises(TypeError, lambda: p.test_unordered_map_conversion(
                {'a': mpf(1), 'b': mpf(2)}))

        mp.prec = 113
        self.assertTrue(p.test_vector_conversion(
            [mpf(1), mpf(-2), mpf("1.1")]) == [mpf(1), mpf(-2), mpf("1.1")])
        self.assertTrue(p.test_vector_conversion(
            [mpc(1, 2), mpc(-2, -3), mpc("1.1")]) == [mpc(1, 2), mpc(-2, -3), mpc("1.1")])
        self.assertTrue(p.test_vector_conversion(
            [mpf(1), mpf(2)**-16495, mpf(3)]) == [mpf(1), mpf(0), mpf(3)])
        self.assertTrue(p.test_vector_conversion(
            [mpc(1), mpc(2)**-16495, mpc(3)]) == [mpc(1), mpc(0), mpc(3)])
        self.assertRaises(TypeError, lambda: p.test_vector_conversion(
            [mpf(1), 2, mpf(3)]))
        self.assertRaises(TypeError, lambda: p.test_vector_conversion(
            [mpc(1), 2, mpc(3)]))

        self.assertTrue(p.test_unordered_map_conversion(
            {'a': mpf("1.1"), 'b': mpf(2)}) == {'a': mpf("1.1"), 'b': mpf(2)})
        self.assertTrue(p.test_unordered_map_conversion(
            {'a': mpc("1.1", 2), 'b': mpc(2)}) == {'a': mpc("1.1", 2), 'b': mpc(2)})
        self.assertTrue(p.test_unordered_map_conversion(
            {'a': mpf(1), 'b': mpf(2)**-16495}) == {'a': mpf(1), 'b': mpf(0)})
        self.assertTrue(p.test_unordered_map_conversion(
            {'a': mpc(1), 'b': mpc(2)**-16495}) == {'a': mpc(1), 'b': mpc(0)})
        self.assertRaises(TypeError, lambda: p.test_unordered_map_conversion(
            {'a': mpf(1), 'b': 2}))
        self.assertRaises(TypeError, lambda: p.test_unordered_map_conversion(
            {'a': mpc(1), 'b': 2}))

        mp.prec = 53

    def run_mpfr_stl(self):
        import pybind11_test_01 as p

        if not p.has_mpfr():
            return

        try:
            from mpmath import mpf, mp, workprec, isnan
        except ImportError:
            return

        self.assertTrue(p.test_vector_conversion(
            [mpf("1.1"), mpf(-2), mpf(3)]) == [mpf("1.1"), mpf(-2), mpf(3)])
        self.assertTrue(p.test_vector_conversion(
            [mpf(1), mpf(2)**-16495, mpf(3)]) == [mpf(1), mpf(2)**-16495, mpf(3)])
        self.assertRaises(TypeError, lambda: p.test_vector_conversion(
            [mpf(1), 2, mpf(3)]))

        self.assertTrue(p.test_unordered_map_conversion(
            {'a': mpf("1.1"), 'b': mpf(2)}) == {'a': mpf("1.1"), 'b': mpf(2)})
        self.assertTrue(p.test_unordered_map_conversion(
            {'a': mpf(1), 'b': mpf(2)**-16495}) == {'a': mpf(1), 'b': mpf(2)**-16495})
        self.assertRaises(TypeError, lambda: p.test_unordered_map_conversion(
            {'a': mpf(1), 'b': 2}))

        with workprec(2000):
            self.assertTrue(p.test_unordered_map_conversion(
                {'a': mpf("1.1"), 'b': mpf(2)}) == {'a': mpf("1.1"), 'b': mpf(2)})
            self.assertTrue(p.test_unordered_map_conversion(
                {'a': mpf(1), 'b': mpf(2)**-16495}) == {'a': mpf(1), 'b': mpf(2)**-16495})
            self.assertRaises(TypeError, lambda: p.test_unordered_map_conversion(
                {'a': mpf(1), 'b': 2}))

    def run_mpc_stl(self):
        import pybind11_test_01 as p

        if not p.has_mpc():
            return

        try:
            from mpmath import mpc, mp, workprec, isnan
        except ImportError:
            return

        self.assertTrue(p.test_vector_conversion(
            [mpc("1.1"), mpc(-2, 1), mpc(3)]) == [mpc("1.1"), mpc(-2, 1), mpc(3)])
        self.assertTrue(p.test_vector_conversion(
            [mpc(1), mpc(2)**-16495, mpc(3, "inf")]) == [mpc(1), mpc(2)**-16495, mpc(3, "inf")])
        self.assertRaises(TypeError, lambda: p.test_vector_conversion(
            [mpc(1), 2, mpc(3)]))

        self.assertTrue(p.test_unordered_map_conversion(
            {'a': mpc("1.1", "inf"), 'b': mpc(2, 3)}) == {'a': mpc("1.1", "inf"), 'b': mpc(2, 3)})
        self.assertTrue(p.test_unordered_map_conversion(
            {'a': mpc(1, -9), 'b': mpc(2)**-16495}) == {'a': mpc(1, -9), 'b': mpc(2)**-16495})
        self.assertRaises(TypeError, lambda: p.test_unordered_map_conversion(
            {'a': mpc(1), 'b': 2}))

        with workprec(2000):
            self.assertTrue(p.test_unordered_map_conversion(
                {'a': mpc("1.1"), 'b': mpc(2, 7)}) == {'a': mpc("1.1"), 'b': mpc(2, 7)})
            self.assertTrue(p.test_unordered_map_conversion(
                {'a': mpc(0, 1), 'b': mpc(2)**-16495}) == {'a': mpc(0, 1), 'b': mpc(2)**-16495})
            self.assertRaises(TypeError, lambda: p.test_unordered_map_conversion(
                {'a': mpc(1), 'b': 2}))

    def test_exceptions(self):
        import pybind11_test_01 as p

        self.assertRaises(ZeroDivisionError,
                          lambda: p.test_zero_division_error())


if __name__ == '__main__':
    unittest.main()
