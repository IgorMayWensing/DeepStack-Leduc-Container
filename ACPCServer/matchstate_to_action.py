# -*- coding: utf-8 -*-
import sys
import math

def extract_info(matchstate):
    # Extract player's cards
    parts = matchstate.split(':')
    
    if len(parts) < 4 or not parts[3]:
        return "Invalid matchstate"

    player_0_cards_parts = parts[3].split('|')
    player_0_cards = player_0_cards_parts[0][0] if player_0_cards_parts and player_0_cards_parts[0] else ""
    
    # Extract the board cards if available
    board_cards = player_0_cards_parts[1].split('/')[0] if len(player_0_cards_parts) > 1 else ""

    # Extract actions and format them
    actions = parts[3].split('/')[0].split('|')[1] if '/' in parts[3] and '|' in parts[3] else ""
    actions = actions.replace('f', 'p').replace('c', 'p')

    # Adjust the raises
    new_actions = []
    for action in actions.split('r'):
        if action:
            if action.isdigit():
                rounded_value = math.ceil(int(action)/100.0) * 100  # Round up to nearest hundred
                new_actions.append('r' + str(rounded_value // 100))
            else:
                new_actions.append(action)
    formatted_actions = ''.join(new_actions)

    # Add "p" to preflop actions if odd
    if formatted_actions.count('p') % 2 == 1:
        formatted_actions += 'p'

    return player_0_cards + board_cards + formatted_actions

if __name__ == "__main__":
    input_string = sys.argv[1]
    result = extract_info(input_string)
    print(result)
