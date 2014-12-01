from ai import AiRequest
import random
class NonDeterministicAi(AiRequest):
    def perfect_play(self, moves):
        sort_by_value_dict = {
            'lose' : 1,
            'tie' : 0,
            'win' : -1,
        }
        moves = sorted(moves, key = lambda move: -move['remoteness'] if move['value'] == 'lose' else move['remoteness'])
        moves = sorted(moves, key = lambda move: sort_by_value_dict[move['value']])
        return (moves[0], "")
    def play(self, moves, prob_perfect = .4, prob_egregious = .5):
        if random.random() < prob_perfect:
            return self.perfect_play(moves)
        else:
            if random.random() < prob_egregious:
                lose_moves = filter(lambda move: move['value'] == 'lose', moves)
                if len(lose_moves) > 0:
                    return (lose_moves[random.randint(0, len(lose_moves) - 1)], "")
            tie_moves = filter(lambda move: move['value'] == 'tie', moves)
            if len(tie_moves) > 0:
                return (tie_moves[random.randint(0, len(tie_moves) - 1)], 0)
            moves = sorted(moves, key = lambda move: -move['remoteness'])
            return (moves[0], 0)
