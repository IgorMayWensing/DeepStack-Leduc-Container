import re
import math
import sys
import pickle
import random
import os

current_file_path = os.path.realpath(__file__)
directory_path = os.path.dirname(current_file_path)


def load_strategy_table():
    # Load the strategy table from the provided pickle file
    with open(f'{directory_path}/../blueprints/DYy42-mccfr-6cards-11maxbet-EPcfr0_0-mRW0_0-iter20000000', 'rb') as file:
        strategy_table = pickle.load(file)
    return strategy_table

STRATEGY_TABLE = load_strategy_table()

def transform_matchstate(matchstate):
    # Divide o matchstate em suas partes
    parts = matchstate.split(":")
    
    # Checa se o matchstate tem o formato requerido
    if len(parts) < 5:
        return "Invalid matchstate format"
    
    cards = ''.join(re.findall(r'[A-Z/]', parts[4]))

    # Extrai o histórico de ações
    action_history = parts[3]
    
    # Divide o histórico de ações com base em "/" para obter ações de cada rodada
    action_rounds = action_history.split("/")
    
    # Função auxiliar para transformar valores de aumento e substituir 'c' ou 'f'
    def transform_action(action):
        
        # Usa regex para encontrar padrões de raise e transformá-los
        def replace_raise(match):
            value = int(match.group(1))
            rounded_value = math.ceil(value / 100) * 100
            return 'r' + str(rounded_value)

        # Substitui 'c' que vem antes de um 'r' por 'k'
        action = re.sub(r'c(?=r)', 'k', action)

        # Substitui 'cc' por 'kk'
        action = re.sub(r'cc', 'kk', action)

        # Substitui 'c' por 'k' se for o único caractere na string
        if action == 'c':
            action = 'k'

        return re.sub(r'r(\d+)', replace_raise, action)
    
    # Transforma cada rodada de ação
    transformed_actions = [transform_action(a) for a in action_rounds]
    # Constrói o resultado final
    action_history = "/".join(transformed_actions)
    
    #add the code here
    first_round = action_history.split("/", 1)[0]
    num_letters = sum(1 for char in first_round if char.isalpha())
    if num_letters % 2 == 1:
        action_history = action_history.replace("/", "-/", 1)

    result = action_history + ":|" + cards
    return result

def test_infoset_str():
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

def action_from_code(action_code, selected_action_index, last_action):
    """Convert an action code to its string representation."""
    if last_action.isdigit():
        dict = {0: 'f',
               1: 'c'}            
    else:
        dict = {0: 'c'} 

    if selected_action_index in dict:
        action = dict[selected_action_index]
    else:
        action = "r" + str(action_code) + "00"
    return action

#r600:|A
#r600c/:|A/A
#r796:|Ah
#cr900c/r1000:|Ks/Qh
#MATCHSTATE:0:1::Ah|

def decide_next_action(infoset):
    # Consult our strategy table
    action_data = STRATEGY_TABLE.get(infoset)
    
    # If we don't have data for this match_state, default to 'c'
    if action_data is None:
        return "c"
    
    actions, probabilities = action_data
    selected_action_code = random.choices(actions, weights=probabilities)[0]

    selected_action_index = actions.index(selected_action_code)
    try:
        last_action = infoset.split(":|")[0][-1]
    except IndexError:
        last_action = 'y'
    return action_from_code(selected_action_code, selected_action_index, last_action)

def main():
    # Get the match state from the command-line arguments
    match_state_str = sys.argv[1]
    #match_state_str = "MATCHSTATE:0:13:cr1174c/cr1200:Ah|/Kh"

    # Parse the match state
    infoset = transform_matchstate(match_state_str)
    #print(infoset)

    # Decide on the next action
    action = decide_next_action(infoset)

    # Print the action to stdout
    print(action)

if __name__ == "__main__":
    main()