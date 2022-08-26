id_0=0
id_1=1
 
curl -X POST localhost:4000/set \
	-H 'Content-Type: application/json' \
	-d '{"global":"articles", "subscripts":["id"], "data":"$id_0"}'