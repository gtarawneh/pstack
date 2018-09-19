def check_log(results):
    """Check number of log messages."""
    assert len(results["log"]) == 40


def check_delivered_messages(results):
    """Check number of delivered messages."""
    assert results["metrics"]["Delivered messages"] == 20


def check_log_order(results):
    """Check order of log messages."""
    device_names = [log_entry[0] for log_entry in results["log"]]
    assert device_names == ["device0", "device1", "device1", "device0"] * 10


def check_states(results):
    """Check final device states."""
    assert results["states"]["device0"] == {"counter": 10, "rts": 0}
    assert results["states"]["device1"] == {"counter": 10, "rts": 0}


def check_exit_code(results):
    """Check exit code is zero."""
    assert results["metrics"]["Exit code"] == 0