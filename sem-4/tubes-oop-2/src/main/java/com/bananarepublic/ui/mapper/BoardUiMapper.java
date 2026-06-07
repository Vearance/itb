package com.bananarepublic.ui.mapper;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.ui.viewmodel.BoardViewModel;
import com.bananarepublic.ui.viewmodel.HexTileViewModel;

public final class BoardUiMapper {
    public BoardViewModel toViewModel(Board board) {
        BoardViewModel vm = new BoardViewModel();
        if (board != null) board.getHexTiles().values().stream().map(HexTileViewModel::new).forEach(vm.getHexTiles()::add);
        return vm;
    }
}
