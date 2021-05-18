#!/usr/bin/env python3

import sys
import subprocess
import tempfile
import pty
import time

class Tests:
    def __init__(self, my_screen):
        self.my_screen = my_screen

    def call(self, args, stdin=b""):
        tmp_in = tempfile.NamedTemporaryFile()
        tmp_in.write(stdin)
        tmp_out = tempfile.NamedTemporaryFile()
        tmp_out_name = tmp_out.name
        tmp_out.close()
        subprocess.run(f"{self.my_screen} {' '.join(args)} < {tmp_in.name} > {tmp_out_name}", shell=True, check=True)
        with open(tmp_out_name, "rb") as f:
            result = f.read()
        print(args, stdin, result)
        tmp_in.close()
        return result

    def list(self):
        result = [i for i in self.call(["list"]).decode().split("\n") if i]
        print(result)
        return result

    def kill(self, session):
        self.call(["kill", session])

    def script(self, script):
        self.call(["new", "1"], script)

    def run(self):
        self.test_session_detach()
        self.test_session_resume()
        self.test_without_session_name()

    def test_session_detach(self):
        self.script(b'echo "test_session_detach"\n')
        assert self.list() == ["1"]
        self.kill("1")
        assert self.list() == []

    def test_session_resume(self):
        self.script(b'echo "test_session_resume1"\n')
        assert self.list() == ["1"]
        self.call(["attach", "1"], b'echo "test_session_resume2"\n')
        assert self.list() == ["1"]
        self.kill("1")
        assert self.list() == []

    def test_without_session_name(self):
        self.call(["new"])
        assert len(self.list()) == 1
        self.call(["new"])
        assert len(self.list()) == 2
        self.call(["new"])
        assert len(self.list()) == 3
        for i in self.list():
            self.kill(i)
        assert len(self.list()) == 0

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <path to my_screen>")
        sys.exit(1)

    tests = Tests(sys.argv[1])
    tests.run()

if __name__ == "__main__":
    main()
