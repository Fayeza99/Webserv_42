# install siege for load teting:
brew install siege
# install python packages for our unit tests
source ./py_env/bin/activate
pip3 install requests

echo "connection = keep-alive" > siege.siegerc
siege -R siege.siegerc http://127.0.0.1:3000 -b -t 10s 

# ----------------------------------------------------------------------------

# some curl commands for testing:

# DELETE a file:
# curl -X DELETE http://localhost:3000/my_files/empty.txt		204
# curl -X DELETE http://localhost:3000/upload/empty.txt			405
# curl -X DELETE http://localhost:3000/my_files/notfound		404

# test max_body_size (set to 10)
# curl -X POST http://localhost:8080/bodysize/post_test.py -d '012345678'		200
# curl -X POST http://localhost:8080/bodysize/post_test.py -d '0123456789'		200
# curl -X POST http://localhost:8080/bodysize/post_test.py -d '01234567890'		413

# bad request
# curl -X UNKNOWN http://localhost:3000/		400