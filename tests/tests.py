import unittest
import requests
import time

# import logging
# logging.basicConfig(level=logging.DEBUG)

# run with: python -m unittest tests.py -v


SERVER_URL = "http://127.0.0.1:3000"

class TestServer(unittest.TestCase):
	def test_a_get_request(self):
		"""simple GET request."""
		response = requests.get(f"{SERVER_URL}/form.html")
		self.assertEqual(response.status_code, 200)

	def test_b_not_found(self):
		"""404"""
		response = requests.get(f"{SERVER_URL}/nonexistent")
		self.assertEqual(response.status_code, 404)
	
	def test_c_post_request(self):
		"""simple POST request"""
		# data = {"name": "Donald Duck", "message": "Hello, World!"}
		data = "name=Donald Duck&message=Hello, World!"
		response = requests.post(f"{SERVER_URL}/cgi/post_test.py", data=data)
		self.assertEqual(response.status_code, 200)
	
	def test_d_file_upload(self):
		"""file upload via POST"""
		path = "fileUpload.txt"
		with open(path, 'rb') as file:
			files = {"file": (path, file.read())}
			response = requests.post(f"{SERVER_URL}/my_files", files=files)
			self.assertEqual(response.status_code, 201)
		# Check if file exists by requesting it
		response = requests.get(f"{SERVER_URL}/upload/{path}")
		self.assertEqual(response.status_code, 200)
	
	def test_e_delete_file(self):
		"""deleting a previously uploaded file."""
		path = "fileUpload.txt"
		response = requests.delete(f"{SERVER_URL}/my_files/{path}")
		self.assertEqual(response.status_code, 204)
		# Ensure file no longer exists
		response = requests.get(f"{SERVER_URL}/upload/{path}")
		self.assertEqual(response.status_code, 404)
	
	def test_f_large_payload(self):
		"""large upload"""
		large_data = {"file": ("bigUpload.txt", "x" * 10000)} # (10KB) check client_max_body_size
		response = requests.post(f"{SERVER_URL}/my_files", files=large_data)
		self.assertEqual(response.status_code, 201) # change to 413 if needed
	
	def test_g_unknown_method(self):
		"""Test an UNKNOWN method"""
		response = requests.request("UNKNOWN", f"{SERVER_URL}/form.html")
		self.assertEqual(response.status_code, 400)
	
	def test_h_directory_listing(self):
		"""directory listing"""
		response = requests.get(f"{SERVER_URL}/assets/css")
		self.assertEqual(response.status_code, 200)
		self.assertIn("/assets/css", response.text)
	
	def test_i_redirection(self):
		"""302 redirect"""
		response = requests.get(f"{SERVER_URL}/redirect", allow_redirects=False)
		self.assertEqual(response.status_code, 302)
		self.assertIn("Location", response.headers, "location not in headers.")
	
	def test_j_loop_timeout(self):
		"""504 CGI timeout"""
		try:
			response = requests.get(f"{SERVER_URL}/cgi/loop.py", timeout=10)
			self.assertEqual(response.status_code, 504)
		except requests.exceptions.Timeout:
			self.fail("Request should have timed out but did not.")

if __name__ == "__main__":
	unittest.main()
