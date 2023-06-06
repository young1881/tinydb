describe 'database' do
  before do
    `rm -rf test.db`
  end

  def run_script(commands)
    raw_output = nil
    IO.popen("./main test.db", "r+") do |pipe|
      commands.each do |command|
        pipe.puts command
      end

      pipe.close_write

      # Read entire output
      raw_output = pipe.gets(nil)
    end
    raw_output.split("\n")
  end

  it 'test exit and unrecognized command' do
    result = run_script([
      "hello world",
      ".HELLO WORLD",
      ".exit",
    ])
    expect(result).to match_array([
      "db > Unrecognized keyword at start of 'hello world'.",
      "db > Unrecognized command: .HELLO WORLD",
      "db > Bye!",
    ])
  end

  it 'inserts and retrieves a row' do
    result = run_script([
      "insert 1 user1 person1@example.com",
      "insert 2 user2",
      "select",
      ".exit",
    ])
    expect(result).to match_array([
      "db > Executed.",
      "db > Syntax error. Could not parse statement.",
      "db > (1, user1, person1@example.com)",
      "Executed.",
      "db > Bye!",
    ])
  end

  it 'prints error message when table is full' do
    script = (1..1401).map do |i|
      "insert #{i} user#{i} person#{i}@example.com"
    end
    script << ".exit"
    result = run_script(script)
    expect(result[-2]).to eq('Error: Table full.')
  end

  it 'allows inserting strings that are the maximum length' do
    long_username = "a"*32
    long_email = "a"*255
    script = [
      "insert 1 #{long_username} #{long_email}",
      "select",
      ".exit",
    ]
    result = run_script(script)
    expect(result).to match_array([
      "db > Executed.",
      "db > (1, #{long_username}, #{long_email})",
      "Executed.",
      "db > Bye!",
    ])
  end

  it 'prints error message if strings are too long' do
    long_username = "a"*33
    long_email = "a"*256
    script = [
      "insert 1 #{long_username} #{long_email}",
      "select",
      ".exit",
    ]
    result = run_script(script)
    expect(result).to match_array([
      "db > String is too long.",
      "db > Executed.",
      "db > Bye!",
    ])
  end

  it 'prints an error message if id is negative' do
    script = [
      "insert -1 cstack foo@bar.com",
      "select",
      ".exit",
    ]
    result = run_script(script)
    expect(result).to match_array([
      "db > ID must be positive.",
      "db > Executed.",
      "db > Bye!",
    ])
  end

  it "keeps data after closing connection" do
    result1 = run_script([
      "insert 1 user1 person1@example.com",
      ".exit",
    ])
    expect(result1).to match_array([
      "db > Executed.",
      "db > Bye!",
    ])
    result2 = run_script([
      "select",
      ".exit",
    ])
    expect(result2).to match_array([
      "db > (1, user1, person1@example.com)",
      "Executed.",
      "db > Bye!",
    ])
  end

  it "allows printing out the structure of a one-node btree" do
    script = [3, 1, 2].map do |i|
      "insert #{i} user#{i} person#{i}@example.com"
    end
    script << ".btree"
    script << ".exit"
    result = run_script(script)

    expect(result).to match_array([
                        "db > Executed.",
                        "db > Executed.",
                        "db > Executed.",
                        "db > Tree:",
                        "leaf (size 3)",
                        "  - 0 : 3",
                        "  - 1 : 1",
                        "  - 2 : 2",
                        "db > Bye!",
                      ])
  end

  it "prints constants" do
    script = [
      ".constants",
      ".exit",
    ]
    result = run_script(script)

    expect(result).to match_array([
                        "db > Constants:",
                        "ROW_SIZE: 293",
                        "COMMON_NODE_HEADER_SIZE: \u0006",
                        "LEAF_NODE_HEADER_SIZE: 10",
                        "LEAF_NODE_CELL_SIZE: 297",
                        "LEAF_NODE_SPACE_FOR_CELLS: 4086",
                        "LEAF_NODE_MAX_CELLS: 13",
                        "db > Bye!",
                      ])
  end
end