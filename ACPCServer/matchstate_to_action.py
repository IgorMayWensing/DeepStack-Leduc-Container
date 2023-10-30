import sys

def parse_match_state(match_state_str):
    print("olha o estado ai pai: " + str(match_state_str))
    return match_state_str

def decide_next_action(match_state):
    # TODO: Implement this function to decide on the next action
    # For now, just return a placeholder action
    return "funcionaaaaaaaaaa"

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