package com.bananarepublic.ui.mapper;

import com.bananarepublic.ui.viewmodel.CardViewModel;
import java.util.List;

public final class CardUiMapper {
    public List<CardViewModel> defaultCards(){
        return List.of(new CardViewModel("Kartu Penjaga", "Pindahkan Nimon Ungu dan curi resource."), new CardViewModel("Kartu Inovasi", "Ambil dua resource pilihan."), new CardViewModel("Kartu Poin Prestasi", "+1 poin tersembunyi."));
    }
}
