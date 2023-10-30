import sys
import math

def round_up_to_nearest_hundred(n):
    return int(math.ceil(n / 100.0)) * 100

def extract_info(matchstate):
    # Separando as partes da string MATCHSTATE
    parts = matchstate.split(':')
    
    # Retirando o naipe e pegando a carta do jogador 0
    player_0_cards = parts[3].split('|')[0][0]
    
    # Retirando o naipe e pegando a carta da mesa (se existir)
    board_cards = parts[3].split('|')[1].split('/')[0][0] if '|' in parts[3] else ''
    
    # O histórico de ações está na terceira parte da string
    action_history = parts[2]
    
    # Fazendo as substituições solicitadas
    action_history = action_history.replace('f', 'p').replace('c', 'p')
    
    # Removendo a letra associada ao "raise" mas mantendo o valor numérico
    while 'r' in action_history:
        index = action_history.index('r')
        if index == len(action_history) - 1 or not action_history[index+1].isdigit():
            action_history = action_history.replace('r', '', 1)
        else:
            # Encontrando o valor do raise e arredondando para a centena mais próxima
            start = index + 1
            end = start
            while end < len(action_history) and action_history[end].isdigit():
                end += 1
            raise_value = int(action_history[start:end])
            rounded_value = round_up_to_nearest_hundred(raise_value)
            
            # Formatando o valor para excluir casas de dezena e unidade
            rounded_value_str = str(rounded_value)[:-2]
            
            action_history = action_history[:index] + rounded_value_str + action_history[end:]

    # Se a quantidade de ações do preflop for ímpar, adicionar a letra "p" no fim do preflop
    preflop_actions = action_history.split('/')[0]
    if len(preflop_actions) % 2 == 1:
        preflop_actions += "p"

    postflop_actions = action_history[len(preflop_actions):] if '/' in action_history else ""

    return f"{player_0_cards}{board_cards}{preflop_actions}{postflop_actions}"

if __name__ == "__main__":
    # Recebendo a entrada
    input_string = sys.argv[1]
    
    # Extraindo as informações desejadas
    result = extract_info(input_string)
    
    # Imprimindo o resultado
    print(result)
