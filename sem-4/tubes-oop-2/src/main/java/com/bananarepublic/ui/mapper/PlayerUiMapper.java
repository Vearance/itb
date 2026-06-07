package com.bananarepublic.ui.mapper;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.service.ScoreService;
import com.bananarepublic.ui.viewmodel.PlayerViewModel;
import java.util.List;
import java.util.stream.Collectors;

public final class PlayerUiMapper {
    private final ScoreService scoreService = new ScoreService();

    public PlayerViewModel toViewModel(AbstractPlayer player, boolean active){ return new PlayerViewModel(player, active); }

    public PlayerViewModel toViewModel(GameState state, Board board, AbstractPlayer player, boolean active) {
        return new PlayerViewModel(player, active, scoreService.calculateTotalPoints(state, board, player));
    }

    public List<PlayerViewModel> toViewModels(List<AbstractPlayer> players, int activeIndex){
        return java.util.stream.IntStream.range(0, players.size()).mapToObj(i -> toViewModel(players.get(i), i == activeIndex)).collect(Collectors.toList());
    }

    public List<PlayerViewModel> toViewModels(GameState state, Board board, int activeIndex) {
        return java.util.stream.IntStream.range(0, state.getAllPlayers().size())
                .mapToObj(i -> toViewModel(state, board, state.getAllPlayers().get(i), i == activeIndex))
                .collect(Collectors.toList());
    }
}
