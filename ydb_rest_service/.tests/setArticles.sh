id_0=0
id_1=1
 
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["$id_0"], "data":"$id_0"}'
	
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["$id_0", "name"], "data":"Tulips"}'
	
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["$id_0", "price"], "data":"3.99"}'
	
