id_tulips=0
id_violets=1
id_roses=2

#Tulips
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global": "articles", "subscripts": ["'"$id_tulips"'"], "data": "'"$id_tulips"'"}'
	
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["'"$id_tulips"'", "name"], "data":"Tulips"}'
	
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["'"$id_tulips"'", "price"], "data":"3.99"}'

#Violets
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global": "articles", "subscripts": ["'"$id_violets"'"], "data": "'"$id_violets"'"}'
	
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["'"$id_violets"'", "name"], "data":"Violets"}'
	
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["'"$id_violets"'", "price"], "data":"2.99"}'

#Roses
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["'"$id_roses"'"], "data":"'"$id_roses"'"}'
	
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["'"$id_roses"'", "name"], "data":"Violets"}'
	
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["'"$id_roses"'", "price"], "data":"4.99"}'
