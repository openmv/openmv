# Tests for finaliser NULL safety in GIF and MJPEG modules
# These tests verify that finalisers don't crash when file_open() fails
import gc

class TestFinaliserSafety:
    def setup(self):
        pass

    def teardown(self):
        gc.collect()

    def test_gif_finaliser_on_failed_open(self):
        """Test that GIF finaliser doesn't crash on failed file_open()"""
        import gif

        # Try to open GIF to invalid path - this will fail
        try:
            g = gif.Gif("/nonexistent/invalid/path.gif")
        except OSError:
            pass  # Expected to fail

        # Force garbage collection to run finaliser
        gc.collect()

        # If we get here without HardFault, the fix works
        assert True, "GIF finaliser survived failed file_open()"

    def test_mjpeg_finaliser_on_failed_open(self):
        """Test that MJPEG finaliser doesn't crash on failed file_open()"""
        import mjpeg

        # Try to open MJPEG to invalid path - this will fail
        try:
            m = mjpeg.Mjpeg("/nonexistent/invalid/path.mjpeg")
        except OSError:
            pass  # Expected to fail

        # Force garbage collection to run finaliser
        gc.collect()

        # If we get here without HardFault, the fix works
        assert True, "MJPEG finaliser survived failed file_open()"

    def test_gif_finaliser_on_readonly_filesystem(self):
        """Test GIF finaliser when trying to write to read-only location"""
        import gif

        # Try to create GIF in location without write permission
        try:
            g = gif.Gif("/flash/readonly_test.gif")
        except (OSError, PermissionError):
            pass  # May fail with different errors depending on filesystem

        gc.collect()
        assert True, "GIF finaliser handled readonly filesystem"

    def test_mjpeg_finaliser_on_readonly_filesystem(self):
        """Test MJPEG finaliser when trying to write to read-only location"""
        import mjpeg

        # Try to create MJPEG in location without write permission
        try:
            m = mjpeg.Mjpeg("/flash/readonly_test.mjpeg")
        except (OSError, PermissionError):
            pass  # May fail with different errors depending on filesystem

        gc.collect()
        assert True, "MJPEG finaliser handled readonly filesystem"

    def test_multiple_failed_opens_with_gc(self):
        """Test that multiple failed opens with GC cycles don't leak or crash"""
        import gif, mjpeg

        for i in range(10):
            # Alternate between GIF and MJPEG failures
            try:
                if i % 2 == 0:
                    g = gif.Gif("/bad/path_%d.gif" % i)
                else:
                    m = mjpeg.Mjpeg("/bad/path_%d.mjpeg" % i)
            except OSError:
                pass

            # Frequent GC to stress test finalisers
            if i % 3 == 0:
                gc.collect()

        # Final GC to clean up any remaining objects
        gc.collect()
        assert True, "Multiple failed opens with GC survived"

    def test_exception_in_constructor_before_file_open(self):
        """Test that early constructor failures are handled safely"""
        import gif

        # This might fail before even reaching file_open()
        # depending on parameter validation
        try:
            g = gif.Gif(None)  # Invalid type
        except (OSError, TypeError, AttributeError):
            pass

        gc.collect()
        assert True, "Early constructor failure handled safely"
