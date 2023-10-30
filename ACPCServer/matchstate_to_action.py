import sys

def parse_match_state(match_state_str):
    print("print do python> estado antigo: " + str(match_state_str) + "\n")

    string_variable = "joguinho"
    print("print do python> estado novo definido para: " + string_variable + "\n")
    return string_variable

def decide_next_action(match_state):
    print("print do python> analise o infostate no nosso jeito\n" )
    # For now, just return a placeholder action
    return "action"

def main():
    # Get the match state from the command-line arguments
    match_state_str = sys.argv[1]

    # Parse the match state
    match_state = parse_match_state(match_state_str)

    # Decide on the next action
    action = decide_next_action(match_state)

    # Print the action to stdout
    print(action)

if __name__ == "__main__":
    main()