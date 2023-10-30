import re

def transform_matchstate(matchstate):
    # Split the matchstate into its constituent parts
    parts = matchstate.split(":")
    
    # Check if the matchstate has the required format
    if len(parts) < 5:
        return "Invalid matchstate format"
    
    # Extract holeCards and remove suits
    holeCards = ''.join([c for c in parts[4].split("|")[1] if c.isalpha() and c.isupper()])
    
    split_parts = parts[4].split("|")
    
    # Extract boardCards and remove suits
    boardCards = ''.join([c for c in split_parts[2] if c.isalpha() and c.isupper()]) if len(split_parts) > 2 else ""
    
    # Extract action history
    action_history = parts[3]
    
    # Split the action history based on "/" to get actions of each round
    action_rounds = action_history.split("/")
    
    # Helper function to transform raise values and replace 'c' or 'f'
    def transform_action(action):
        # Replace "f" with "p"
        action = action.replace("f", "p")
        
        # Use regex to find raise patterns and transform them
        def replace_raise(match):
            value = int(match.group(1))
            rounded_value = (value + 99) // 100
            hex_value = hex(rounded_value)[2:]
            return hex_value

        return re.sub(r'r(\d+)', replace_raise, action)
    
    # Transform each round of action
    transformed_actions = [transform_action(a) for a in action_rounds]
    
    # Construct the final result
    result = holeCards + boardCards + "".join(transformed_actions)
    
    return result

# Test
test_cases = [
    "MATCHSTATE:1:0::|Ks",
    "MATCHSTATE:1:0:c:|Ks",
    "MATCHSTATE:1:0:cc/:|Ks/Qh",
    "MATCHSTATE:1:0:cc/c:|Ks/Qh",
    "MATCHSTATE:1:0:cc/cc:Kh|Ks/Qh",
    "MATCHSTATE:1:1::|Qs",
    "MATCHSTATE:1:1:r451:|Qs",
    "MATCHSTATE:1:1:r451c/:|Qs/Kh",
    "MATCHSTATE:1:1:r451c/c:|Qs/Kh",
    "MATCHSTATE:1:1:r451c/cc:Ah|Qs/Kh",
    "MATCHSTATE:1:2::|Kh",
    "MATCHSTATE:1:2:c:|Kh",
    "MATCHSTATE:1:2:cr369:|Kh",
    "MATCHSTATE:1:2:cr369r1200:|Kh",
    "MATCHSTATE:1:2:cr369r1200c/:As|Kh/Qs",
    "MATCHSTATE:1:3::|Qs",
    "MATCHSTATE:1:3:c:|Qs",
    "MATCHSTATE:1:3:cc/:|Qs/Kh",
    "MATCHSTATE:1:3:cc/c:|Qs/Kh",
    "MATCHSTATE:1:3:cc/cc:Qh|Qs/Kh",
    "MATCHSTATE:1:4::|Qs",
    "MATCHSTATE:1:4:c:|Qs",
    "MATCHSTATE:1:4:cr932:|Qs",
    "MATCHSTATE:1:4:cr932f:|Qs",
    "MATCHSTATE:1:5::|Ah",
    "MATCHSTATE:1:5:r578:|Ah",
    "MATCHSTATE:1:5:r578r1200:|Ah",
    "MATCHSTATE:1:5:r578r1200c/:Qh|Ah/Kh",
    "MATCHSTATE:1:6::|Kh",
    "MATCHSTATE:1:6:r1200:|Kh",
    "MATCHSTATE:1:6:r1200f:|Kh",
    "MATCHSTATE:1:7::|Kh",
    "MATCHSTATE:1:7:r235:|Kh",
    "MATCHSTATE:1:7:r235c/:|Kh/Qh",
    "MATCHSTATE:1:7:r235c/c:|Kh/Qh",
    "MATCHSTATE:1:7:r235c/cc:Qs|Kh/Qh",
    "MATCHSTATE:1:8::|Kh",
    "MATCHSTATE:1:8:c:|Kh",
    "MATCHSTATE:1:8:cr552:|Kh",
    "MATCHSTATE:1:8:cr552f:|Kh",
    "MATCHSTATE:1:9::|Ks"
]

for test in test_cases:
    print(transform_matchstate(test))
