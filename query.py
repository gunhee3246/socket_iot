from influxdb import InfluxDBClient
from socket import *
import pickle,time
def main():
    """Instantiate a connection to the InfluxDB."""
    host = 'localhost'
    port = 8086
    user = 'admin'
    password = 'admin'
    dbname = 'test3'
    query = 'select * from light order by time desc limit 20'    
    serverSock = socket(AF_INET, SOCK_STREAM)
    serverSock.bind(('', 9051))
    serverSock.listen(1)
    connectionSock, addr = serverSock.accept()
    while True:
        aver=0
        total=0
        client = InfluxDBClient(host, port, user, password, dbname)

        print("Querying data: " + query)
        result = client.query(query)

        for point in result.get_points():
            print(point)
            total += point['data']

        print("total : ", (total))
        aver = total/20
        print("average : ", (aver))
        data = pickle.dumps(aver)
        connectionSock.sendall(data)
        #connectionSock.send(round(aver).encode('utf-8'))
        print('메시지를 보냈습니다.')
        time.sleep(2)

if __name__ == '__main__':
    main()