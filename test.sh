g++ *.cpp -o main
rspec spec test_db.rb

# test memory operation
# valgrind --leak-check=yes ./db test.db