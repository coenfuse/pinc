import snap7
import time
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

# Class representing each machine
class Machine:
    def __init__(self, id: str):
        self.__id = id
        self.__is_on = False
        self.__start = None
        self.__stop = None
        self.__run_time = 0
        self.__down_time = 0

    def turn_on(self):
        if not self.__is_on:
            self.__is_on = True
            self.__start = time.time()

            if self.__stop:
                # Calculate downtime when transitioning from off to on
                self.__down_time = self.__start - self.__stop
                print(f"Machine {self.__id} turned on after being off for {self.__down_time:.2f} seconds")
            
            else:
                print(f"Machine {self.__id} turned on")

    def turn_off(self):
        if self.__is_on:
            self.__is_on = False
            # Calculate the time machine was running
            self.__run_time = time.time() - self.__start
            self.__stop = time.time()
            print(f"Machine {self.__id} turned off after running for {self.__run_time:.2f} seconds")
            return self.__run_time
        return 0

    def get_runtime(self):
        return self.__run_time

    def get_downtime(self):
        return self.__down_time

    def get_id(self):
        return self.__id

    def is_on(self):
        return self.__is_on


# Function to convert PLC data to 8-bit binary array
def get_8bit_binary_array(num):
    binary_string = bin(num)[2:].zfill(8)
    return [int(bit) for bit in binary_string]


# InfluxDB setup
def influxdb_setup():
    bucket = "machines"
    org = "your-org"
    token = "your-token"
    url = "http://localhost:8086"

    # Create a client
    client = InfluxDBClient(url=url, token=token, org=org)
    write_api = client.write_api(write_options=SYNCHRONOUS)

    return client, write_api, bucket, org


# Function to write runtime data to InfluxDB
def write_to_influx(write_api, bucket, org, machine_id, runtime=None, downtime=None):
    point = Point("machine_status") \
        .tag("machine_id", machine_id) \
        .time(time.time_ns(), WritePrecision.NS)
    
    if runtime is not None:
        point.field("runtime", runtime)
        print(f"Runtime for Machine {machine_id} written to InfluxDB: {runtime:.2f} seconds")

    if downtime is not None:
        point.field("downtime", downtime)
        print(f"Downtime for Machine {machine_id} written to InfluxDB: {downtime:.2f} seconds")

    write_api.write(bucket=bucket, org=org, record=point)


def main():
    # PLC client setup
    client = snap7.client.Client()
    client.connect("192.168.0.40", 0, 0, 102)

    # InfluxDB client setup
    influx_client, write_api, bucket, org = influxdb_setup()

    # Initialize 8 machines
    machines_array = [Machine(i + 1) for i in range(8)]

    try:
        while True:
            # Read status from PLC
            status = get_8bit_binary_array(client.eb_read(0, 1)[0])  # Assuming eb_read returns byte buffer

            for index, is_on in enumerate(status):
                machine = machines_array[index]

                if is_on:
                    # If machine turns on, log the downtime (if any) before starting the machine
                    if not machine.is_on():
                        downtime = machine.get_downtime()
                        if downtime > 0:
                            write_to_influx(write_api, bucket, org, machine.get_id(), downtime=downtime)
                    machine.turn_on()
                
                else:
                    # If machine turns off, log the runtime
                    runtime = machine.turn_off()
                    if runtime > 0:
                        write_to_influx(write_api, bucket, org, machine.get_id(), runtime=runtime)

            time.sleep(1)  # Poll every second

    except KeyboardInterrupt:
        print("Program terminated by user")

    finally:
        influx_client.close()
        client.disconnect()
        print("InfluxDB and PLC connections closed")


if __name__ == "__main__":
    main()