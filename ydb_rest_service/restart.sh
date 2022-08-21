sudo docker kill ydb_rest
sudo docker build -t ydb_rest_service .
sudo docker run --rm --name "ydb_rest" -d -p 4000:4000 ydb_rest_service
