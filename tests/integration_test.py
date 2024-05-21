import sys
import subprocess
import requests
import time
import socket
import traceback

class webserver_fixture:
    def __init__(self, location):
        self.location = location
        if self.location != 'localhost:8080':
            return
        command = [f'{sys.path[0]}/../build/bin/webserver',
                   f'{sys.path[0]}/../server_config']
        self.__webserver = subprocess.Popen(command,
                                            stdout=subprocess.DEVNULL,
                                            stderr=subprocess.DEVNULL)
        while True:
            try:
                socket_status = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                socket_status.connect(('localhost', 8080))
                connected = True
                break
            except ConnectionError:
                time.sleep(0.1)
            finally:
                socket_status.close()

    def cleanup(self):
        self.__webserver.kill()


class local_tests(webserver_fixture):
    def __init__(self):
        super().__init__('localhost:8080');
        self.url = f'http://{self.location}'

    def run_test(self, TEST):
        assert(hasattr(self, TEST))
        method = getattr(self, TEST)
        assert(callable(method))
        method()


    def test_crud_put_retrieve_delete(self):
        data = '{"test":"json-blob"}'
        put_response = requests.request('PUT', f'{self.url}/api/Test/1', data=data)
        assert put_response.status_code == 200
        get_response = requests.request('GET', f'{self.url}/api/Test/1')
        assert get_response.status_code == 200
        assert get_response.text == data
        delete_response = requests.request('DELETE', f'{self.url}/api/Test/1')
        assert delete_response.status_code == 200
        bad_get = requests.request('GET', f'{self.url}/api/Test/1')
        assert bad_get.status_code == 404

    def test_crud_post_list_delete(self):
        data = '{"post-test":"another json-blob"}'
        post_response = requests.request('POST', f'{self.url}/api/Test', data=data)
        json_response = post_response.json()
        assert post_response.status_code == 200
        list_response = requests.request('GET', f'{self.url}/api/Test')
        assert list_response.status_code == 200
        assert list_response.text == f'[{json_response["id"]}]'
        delete_response = requests.request('DELETE', f'{self.url}/api/Test/{json_response["id"]}')
        assert delete_response.status_code == 200


# This is the entrypoint for all tests. When running the python program like
# `python3 integration_test.py [TEST_FUNCTION]` the [TEST_FUNCTION] is evidently
# a function name which corresponds to a test. Each test function must be
# prefixed with `test_` in order to get collected by CMake but otherwise if
# running manually then there are no restrictions beyond what is enforced before
# calling.
# Currently there is only 1 testing class which is setup for testing a local
# webserver instance by inheriting the webserver fixture class which starts up
# the webserver and ensures it is properly running before commencing the test.
# All assertions will immediately stop test execution and propogate to the main
# try except where the test will be marked as failing (exit status 1) and
# the webserver will be cleaned up
#
# NOTE: With testing which has the possibility to leak state between tests
# (as is the case when persistent storage is a requirement) tests can be
# unknowingly coupled, it may be a good idea to have an impermanent directory
# for crud testing which gets nuked before every test
if __name__ == "__main__":
    TEST = sys.argv[1]
    try:
        test_object = local_tests()
        test_object.run_test(TEST)
        print(f'Test [{TEST}] PASSED')
    except Exception as e:
        print(f'Test [{TEST}] FAILED')
        traceback.print_exc()
        sys.exit(1)
    finally:
        print('cleaning up webserver')
        test_object.cleanup()
