import time 
from concurrent.futures import ThreadPoolExecutor , as_completed
import requests

from test_multiple_reader import make_request
from write_test import write_request

def main():
    ncycles = 3
    start = time.perf_counter()
    read_thread_id = 1
    write_thread_id = 1
    with ThreadPoolExecutor() as executor:
        for turn in range(ncycles):
            cycle_start = time.perf_counter()
            results = [executor.submit(make_request, id) for id in range(1, 5)]
            time_list = [executor.submit(write_request , ["Woww\n"])]

            for f in as_completed(results):
                print(f"Reader thread {read_thread_id} execution time taken = ", f.result())
                read_thread_id += 1
            for i in as_completed(time_list):
                print(f"Writer thread {write_thread_id} execution time taken = ", i.result())
                write_thread_id += 1
            
            cycle_end = time.perf_counter()
            print("total time taken for current cycle ", cycle_end - cycle_start)
            print ("---------------##########New Cycle##########--------------------")


    end = time.perf_counter()

    print("total time taken ", end - start)

if __name__=="__main__":
    main()
