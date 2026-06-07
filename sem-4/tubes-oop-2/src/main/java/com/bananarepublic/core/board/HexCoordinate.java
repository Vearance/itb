package com.bananarepublic.core.board;

public record HexCoordinate(int q, int r) {
    // hexagonal coordinates use three coordinates (q, r, s) where q + r + s = 0
    // read more: https://www.redblobgames.com/grids/hexagons/#coordinates-cube
    public int s() {
        return -q - r;
    }
}
