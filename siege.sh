brew install siege
echo "connection = keep-alive" > siege.siegerc
siege -R siege.siegerc http://127.0.0.1:3000 -b -t 10s 