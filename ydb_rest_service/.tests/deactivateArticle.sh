article_id=0

curl -X POST localhost:4000/deactivateArticle \
	-H 'Content-Type: application/json' \
	-d '{"articleId":"'"$article_id"'"}'