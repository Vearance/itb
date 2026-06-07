package com.bananarepublic.ui.controller;

import com.bananarepublic.app.AppContext;
import com.bananarepublic.core.action.MoveRobberAction;
import com.bananarepublic.core.action.TradeAction;
import com.bananarepublic.core.board.*;
import com.bananarepublic.core.dice.DiceRoll;
import com.bananarepublic.core.dice.ManualDiceRoller;
import com.bananarepublic.core.dice.RandomDiceRoller;
import com.bananarepublic.core.board.generator.StandardBoardGenerator;
import com.bananarepublic.core.card.OwnedExperimentCard;
import com.bananarepublic.core.game.GameEngine;
import com.bananarepublic.core.game.GamePhase;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.game.GameStatus;
import com.bananarepublic.core.game.TurnManager;
import com.bananarepublic.core.game.VictoryManager;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.BotPlayer;
import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.core.resource.ResourceBundle;
import com.bananarepublic.core.resource.ResourceType;
import com.bananarepublic.event.BuildingPlacedEvent;
import com.bananarepublic.event.BuildingUpgradedEvent;
import com.bananarepublic.event.CardBoughtEvent;
import com.bananarepublic.event.CardPlayedEvent;
import com.bananarepublic.event.GameEvent;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.GameWonEvent;
import com.bananarepublic.event.PhaseChangedEvent;
import com.bananarepublic.event.PlayerTurnEndedEvent;
import com.bananarepublic.event.PlayerTurnStartedEvent;
import com.bananarepublic.event.ResourceChangedEvent;
import com.bananarepublic.event.ResourceDiscardedEvent;
import com.bananarepublic.event.ResourceProducedEvent;
import com.bananarepublic.event.ResourceStolenEvent;
import com.bananarepublic.event.RobberMovedEvent;
import com.bananarepublic.event.TimerExpiredEvent;
import com.bananarepublic.event.TimerStartedEvent;
import com.bananarepublic.event.TimerTickEvent;
import com.bananarepublic.event.TradeCompletedEvent;
import com.bananarepublic.event.TradeRejectedEvent;
import com.bananarepublic.event.TradeRequestedEvent;
import com.bananarepublic.service.BuildService;
import com.bananarepublic.service.BotTurnReport;
import com.bananarepublic.service.BotTurnService;
import com.bananarepublic.service.DiscardService;
import com.bananarepublic.service.DiceService;
import com.bananarepublic.service.InitialSetupService;
import com.bananarepublic.service.ResourceProductionService;
import com.bananarepublic.service.RobberService;
import com.bananarepublic.service.CardService;
import com.bananarepublic.service.ScoreService;
import com.bananarepublic.service.TradeService;
import com.bananarepublic.timer.TimerState;
import com.bananarepublic.timer.TurnTimer;
import com.bananarepublic.ui.audio.BackgroundMusicPlayer;
import com.bananarepublic.ui.audio.SoundEffectPlayer;
import com.bananarepublic.ui.component.*;
import com.bananarepublic.ui.listener.GameUiEventBridge;
import com.bananarepublic.ui.mapper.PlayerUiMapper;
import com.bananarepublic.ui.mapper.ResourceUiMapper;
import com.bananarepublic.ui.util.AlertUtil;
import com.bananarepublic.ui.util.FxThreadUtil;
import com.bananarepublic.ui.viewmodel.GameViewModel;
import javafx.application.Platform;
import javafx.fxml.FXML;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.scene.shape.Line;
import javafx.scene.shape.Polygon;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;

import java.util.ArrayList;
import java.util.List;

public final class GameController {
    private enum BuildMode { NONE, SETTLEMENT, PIPE, LABORATORY }
    private record InitialPlacementChoice(IntersectionId intersectionId, PathId pathId) {}
    private static final String HUD_FONT_RESOURCE = "/fonts/banana-republic-hud.ttf";
    private static final int ROAD_BUILDING_PIPE_COUNT = 2;

    @FXML private BorderPane root;
    @FXML private VBox boardHud;
    @FXML private VBox playerPanel;
    @FXML private FlowPane bankResourcePanel;
    @FXML private HBox resourcePanel;
    @FXML private TextArea logArea;
    @FXML private Label currentPlayerLabel;
    @FXML private Label roundLabel;
    @FXML private Label phaseLabel;
    @FXML private BoardView boardView;
    @FXML private DiceView diceView;
    @FXML private TimerView timerView;
    @FXML private Button rollDiceButton;
    @FXML private Button manualDiceButton;
    @FXML private Button buildSettlementButton;
    @FXML private Button buildPipeButton;
    @FXML private Button upgradeLabButton;
    @FXML private Button tradeButton;
    @FXML private Button cardsButton;
    @FXML private Button buyNeroifaCardButton;
    @FXML private Button settingsButton;
    @FXML private Button endTurnButton;

    private GameViewModel viewModel;
    private Board board;
    private BuildMode buildMode = BuildMode.NONE;
    private IntersectionId pendingInitialSettlementId;
    private boolean victoryDialogAlreadyShown;
    private boolean botTurnRunning;
    private boolean botTurnScheduled;

    @FXML
    private void initialize() {
        viewModel = getGameViewModel();
        GameState state = getState();
        board = AppContext.getCurrent().serviceRegistry().contains(Board.class)
                ? AppContext.getCurrent().serviceRegistry().get(Board.class)
                : new StandardBoardGenerator().generate().board();
        AppContext.getCurrent().serviceRegistry().register(Board.class, board);
        getGameEngine();
        initializePhase(state);
        startBackgroundMusicIfGameActive(state);
        configureActionButtons();

        if (boardView != null) {
            boardView.setOnIntersectionSelected(this::handleIntersectionSelected);
            boardView.setOnPathSelected(this::handlePathSelected);
            boardView.setOnHexTileSelected(this::handleHexTileSelected);
            boardView.setBoard(board, state.getAllPlayers());
        }
        if (timerView != null) timerView.bind(viewModel.getTimer());
        if (logArea != null) logArea.textProperty().bind(viewModel.logTextProperty());
        bindEventBusToUi();
        applyHudFont();
        refresh(state);
        evaluateAutomaticGameEvents();
        if (!viewModel.hasLog()) {
            viewModel.appendLog(isInitialSetupPhase(state) ? "Permainan dimulai: setup awal." : "Permainan dilanjutkan.");
        }
        scheduleBotTurnIfNeeded(state);
    }

    private GameViewModel getGameViewModel() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(GameViewModel.class)) {
            return registry.get(GameViewModel.class);
        }

        GameViewModel vm = new GameViewModel();
        registry.register(GameViewModel.class, vm);
        return vm;
    }

    private GameState getState() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (!registry.contains(GameState.class)) {
            registry.register(GameState.class, GameState.newGame(
                    com.bananarepublic.core.player.PlayerFactory.createDefaultHumanPlayers(java.util.List.of("Kebin", "Stewart", "Gro"))
            ));
        }
        return registry.get(GameState.class);
    }

    private GameEngine getGameEngine() {
        var registry = AppContext.getCurrent().serviceRegistry();
        GameState state = getState();
        if (board == null) {
            board = registry.contains(Board.class)
                    ? registry.get(Board.class)
                    : new StandardBoardGenerator().generate().board();
            registry.register(Board.class, board);
        }

        if (registry.contains(GameEngine.class)) {
            GameEngine engine = registry.get(GameEngine.class);
            if (engine.getState() == state && engine.getBoard() == board) {
                return engine;
            }
        }

        GameEventBus eventBus = getEventBus();
        GameEngine engine = new GameEngine(
                state,
                board,
                getInitialSetupService(),
                getBuildService(),
                getTurnManager(),
                getVictoryManager(),
                getDiceService(),
                getResourceProductionService(),
                getTradeService(),
                getCardService(),
                getDiscardService(),
                getRobberService(),
                eventBus
        );
        registry.register(GameEngine.class, engine);
        return engine;
    }

    private void bindEventBusToUi() {
        getGameUiEventBridge().bind(event -> FxThreadUtil.runOnFxThread(() -> handleGameEvent(event)));
    }

    private GameUiEventBridge getGameUiEventBridge() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(GameUiEventBridge.class)) {
            return registry.get(GameUiEventBridge.class);
        }

        GameUiEventBridge bridge = GameUiEventBridge.registerTo(getEventBus());
        registry.register(GameUiEventBridge.class, bridge);
        return bridge;
    }

    private void handleGameEvent(GameEvent event) {
        if (event instanceof TimerStartedEvent timerStartedEvent) {
            handleTimerStartedEvent(timerStartedEvent);
            return;
        }
        if (event instanceof TimerTickEvent timerTickEvent) {
            handleTimerTickEvent(timerTickEvent);
            return;
        }
        if (event instanceof TimerExpiredEvent timerExpiredEvent) {
            handleTimerExpiredEvent(timerExpiredEvent);
            return;
        }
        if (event instanceof GameWonEvent) {
            refresh(getState());
            showVictoryDialog(getState());
            return;
        }
        if (event instanceof TradeRequestedEvent tradeRequestedEvent) {
            appendTradeRequestedLog(tradeRequestedEvent);
        } else if (event instanceof TradeRejectedEvent tradeRejectedEvent) {
            appendTradeRejectedLog(tradeRejectedEvent);
        } else if (event instanceof TradeCompletedEvent tradeCompletedEvent) {
            appendTradeCompletedLog(tradeCompletedEvent);
        }
        if (shouldRefreshAfter(event)) {
            refresh(getState());
        }
    }

    private void handleTimerStartedEvent(TimerStartedEvent event) {
        if (!isCurrentTimerEvent(event.playerId(), event.turnNumber())) {
            return;
        }
        GameState state = getState();
        state.setTurnTimerRunning(true);
        state.setTurnTimerRemainingSeconds(event.durationSeconds());
        viewModel.getTimer().setRunning(true);
        viewModel.getTimer().setRemainingSeconds(event.durationSeconds());
    }

    private void handleTimerTickEvent(TimerTickEvent event) {
        if (!isCurrentTimerEvent(event.playerId(), event.turnNumber())) {
            return;
        }
        getState().setTurnTimerRemainingSeconds(event.remainingSeconds());
        viewModel.getTimer().setRemainingSeconds(event.remainingSeconds());
    }

    private void handleTimerExpiredEvent(TimerExpiredEvent event) {
        if (!isCurrentTimerEvent(event.playerId(), event.turnNumber())) {
            return;
        }
        getState().setTurnTimerRunning(false);
        viewModel.getTimer().setRunning(false);
        viewModel.getTimer().setRemainingSeconds(0);
    }

    private boolean isCurrentTimerEvent(PlayerId playerId, int turnNumber) {
        GameState state = getState();
        return state.getCurrentPlayer().getId().equals(playerId)
                && state.getTurnNumber() == turnNumber;
    }

    private boolean shouldRefreshAfter(GameEvent event) {
        return event instanceof BuildingPlacedEvent
                || event instanceof BuildingUpgradedEvent
                || event instanceof CardBoughtEvent
                || event instanceof CardPlayedEvent
                || event instanceof PhaseChangedEvent
                || event instanceof PlayerTurnEndedEvent
                || event instanceof PlayerTurnStartedEvent
                || event instanceof ResourceChangedEvent
                || event instanceof ResourceDiscardedEvent
                || event instanceof ResourceProducedEvent
                || event instanceof ResourceStolenEvent
                || event instanceof RobberMovedEvent
                || event instanceof TradeCompletedEvent;
    }

    private void appendTradeRequestedLog(TradeRequestedEvent event) {
        TradeAction action = event.tradeAction();
        viewModel.appendLog(playerName(action.getInitiatorId())
                + " menawarkan trade ke "
                + playerName(action.getTargetId())
                + ": memberi "
                + resourceBundleText(action.getOfferedResources())
                + ", meminta "
                + resourceBundleText(action.getRequestedResources())
                + ".");
    }

    private void appendTradeRejectedLog(TradeRejectedEvent event) {
        if ("Counter-offer proposed".equals(event.reason())) {
            return;
        }

        TradeAction action = event.tradeAction();
        viewModel.appendLog("Trade "
                + playerName(action.getInitiatorId())
                + " ke "
                + playerName(action.getTargetId())
                + " dibatalkan: "
                + event.reason()
                + ".");
    }

    private void appendTradeCompletedLog(TradeCompletedEvent event) {
        String playerName = playerName(event.activePlayerId());
        String given = resourceBundleText(event.givenResources());
        String received = resourceBundleText(event.receivedResources());

        if (event.harborTrade()) {
            viewModel.appendLog(playerName + " trade dengan bank: memberi " + given + ", menerima " + received + ".");
            return;
        }

        String partnerName = event.partnerPlayerId().map(this::playerName).orElse("Bank");
        viewModel.appendLog("Trade selesai: "
                + playerName
                + " memberi "
                + given
                + " ke "
                + partnerName
                + " dan menerima "
                + received
                + ".");
    }

    private String playerName(PlayerId playerId) {
        return getState()
                .findPlayerById(playerId)
                .map(AbstractPlayer::getName)
                .orElse(playerId.getValue());
    }

    private String resourceBundleText(ResourceBundle bundle) {
        List<String> parts = new ArrayList<>();
        for (ResourceType type : ResourceType.values()) {
            int amount = bundle.get(type);
            if (amount > 0) {
                parts.add(amount + " " + ResourceIconFactory.display(type));
            }
        }
        return parts.isEmpty() ? "0 resource" : String.join(", ", parts);
    }

    private void initializePhase(GameState state) {
        if (state.getPhase() == GamePhase.DETERMINE_START_PLAYER) {
            state.setStatus(GameStatus.NOT_STARTED);
            state.resetInitialPlacementProgress();
            getGameEngine().rollStartingPlayer(new RandomDiceRoller());
            buildMode = BuildMode.SETTLEMENT;
            return;
        }

        if (isInitialSetupPhase(state)) {
            state.setStatus(GameStatus.NOT_STARTED);
            buildMode = pendingInitialSettlementId == null ? BuildMode.SETTLEMENT : BuildMode.PIPE;
            return;
        }

        if (state.getStatus() != GameStatus.GAME_OVER) {
            state.setStatus(GameStatus.IN_PROGRESS);
        }
    }

    private TurnTimer getTurnTimer() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(TurnTimer.class)) {
            return registry.get(TurnTimer.class);
        }

        TurnTimer timer = new TurnTimer(getEventBus());
        registry.register(TurnTimer.class, timer);
        return timer;
    }

    private void startTurnTimer(GameState state) {
        if (!isTimedTurnPhase(state)) {
            return;
        }
        int remainingSeconds = state.getTurnTimerRemainingSeconds() > 0
                ? state.getTurnTimerRemainingSeconds()
                : TurnTimer.ROUND_DURATION;
        state.setTurnTimerRunning(true);
        state.setTurnTimerRemainingSeconds(remainingSeconds);
        viewModel.getTimer().setRunning(true);
        viewModel.getTimer().setRemainingSeconds(remainingSeconds);
        getTurnTimer().start(
                state.getCurrentPlayer().getId(),
                state.getTurnNumber(),
                remainingSeconds,
                secondsLeft -> FxThreadUtil.runOnFxThread(() -> {
                    state.setTurnTimerRemainingSeconds(secondsLeft);
                    viewModel.getTimer().setRemainingSeconds(secondsLeft);
                }),
                () -> FxThreadUtil.runOnFxThread(this::handleTimerExpired)
        );
    }

    private void stopTurnTimer(GameState state, boolean resetRemaining) {
        getTurnTimer().stop();
        state.setTurnTimerRunning(false);
        if (resetRemaining) {
            state.setTurnTimerRemainingSeconds(TurnTimer.ROUND_DURATION);
            viewModel.getTimer().setRemainingSeconds(TurnTimer.ROUND_DURATION);
        }
        viewModel.getTimer().setRunning(false);
    }

    private void syncTimerWithPhase(GameState state) {
        TurnTimer timer = getTurnTimer();
        boolean timerRunning = timer.getState() == TimerState.RUNNING;

        if (isCurrentPlayerBot(state)) {
            if (timerRunning || state.isTurnTimerRunning()) {
                stopTurnTimer(state, true);
            }
            viewModel.getTimer().setRunning(false);
            viewModel.getTimer().setRemainingSeconds(TurnTimer.ROUND_DURATION);
            return;
        }

        if (isTimedTurnPhase(state)) {
            if (!timerRunning) {
                startTurnTimer(state);
            }
            return;
        }

        if (timerRunning || state.isTurnTimerRunning()) {
            stopTurnTimer(state, true);
        } else {
            state.setTurnTimerRemainingSeconds(TurnTimer.ROUND_DURATION);
            viewModel.getTimer().setRemainingSeconds(TurnTimer.ROUND_DURATION);
            viewModel.getTimer().setRunning(false);
        }
    }

    private void handleTimerExpired() {
        GameState state = getState();
        if (!isPlayerActionPhase(state)) {
            if (state.getPhase() == GamePhase.MOVE_ROBBER) {
                viewModel.appendLog("Timer habis. Pindahkan Nimon Ungu untuk melanjutkan giliran.");
            }
            stopTurnTimer(state, true);
            return;
        }
        if (hasPendingRoadBuildingCardSelection()) {
            viewModel.appendLog("Timer habis. Selesaikan Road Building untuk melanjutkan giliran.");
            stopTurnTimer(state, true);
            return;
        }

        viewModel.appendLog("Timer habis. Giliran berpindah otomatis.");
        stopTurnTimer(state, true);
        advanceTurnAndShowTransition();
    }

    private void autoCompleteInitialSetupTurn() {
        if (pendingInitialSettlementId != null) {
            PathId pathId = findAvailableInitialPipe(pendingInitialSettlementId);
            if (pathId != null) {
                viewModel.getTimer().setRemainingSeconds(TurnTimer.ROUND_DURATION);
                placeInitialSettlementAndPipe(pathId);
                return;
            }

            pendingInitialSettlementId = null;
            if (boardView != null) {
                boardView.clearPendingSettlement();
            }
        }

        InitialPlacementChoice choice = findAutomaticInitialPlacement();
        if (choice == null) {
            viewModel.appendLog("Sistem gagal menemukan setup otomatis yang valid.");
            viewModel.getTimer().setRemainingSeconds(TurnTimer.ROUND_DURATION);
            return;
        }

        pendingInitialSettlementId = choice.intersectionId();
        if (boardView != null) {
            boardView.setPendingSettlement(choice.intersectionId(), getState().getCurrentPlayer().getId());
        }
        viewModel.getTimer().setRemainingSeconds(TurnTimer.ROUND_DURATION);
        placeInitialSettlementAndPipe(choice.pathId());
    }

    private InitialPlacementChoice findAutomaticInitialPlacement() {
        for (Intersection intersection : board.getIntersections().values()) {
            if (intersection.hasBuilding()) {
                continue;
            }
            try {
                requireDistanceRule(intersection.getId());
            } catch (RuntimeException ignored) {
                continue;
            }

            PathId pathId = findAvailableInitialPipe(intersection.getId());
            if (pathId != null) {
                return new InitialPlacementChoice(intersection.getId(), pathId);
            }
        }
        return null;
    }

    private PathId findAvailableInitialPipe(IntersectionId intersectionId) {
        return board.getPaths().values().stream()
                .filter(path -> path.touches(intersectionId))
                .filter(path -> !path.hasPipe())
                .map(Path::getId)
                .findFirst()
                .orElse(null);
    }

    private void refresh(GameState state) {
        viewModel.setCurrentPlayerName(state.getCurrentPlayer().getName());
        viewModel.setPhase(state.getPhase());
        if (currentPlayerLabel != null) currentPlayerLabel.setText(String.valueOf(visibleTurnNumber(state)));
        if (roundLabel != null) roundLabel.setText(turnTypeText(state));
        if (phaseLabel != null) phaseLabel.setText("");
        refreshBankResourcePanel(state);
        playerPanel.getChildren().clear();
        new PlayerUiMapper().toViewModels(state, board, state.getCurrentPlayerIndex())
                .forEach(vm -> playerPanel.getChildren().add(new PlayerPanelView(vm)));
        resourcePanel.getChildren().clear();
        new ResourceUiMapper().toViewModels(state.getCurrentPlayer().getInventory())
                .forEach(vm -> resourcePanel.getChildren().add(new ResourceCardView(vm)));
        if (boardView != null) boardView.refresh();
        refreshPieceActionButtonIcons(state);
        refreshActionAvailability(state);
        syncTimerWithPhase(state);
        scheduleBotTurnIfNeeded(state);
    }

    private void refreshBankResourcePanel(GameState state) {
        if (bankResourcePanel == null) {
            return;
        }
        bankResourcePanel.getChildren().clear();
        for (ResourceType type : ResourceType.values()) {
            bankResourcePanel.getChildren().add(createBankChip(
                    ResourceIconFactory.iconForResource(type, 22),
                    state.getBank().getCount(type)));
        }
        bankResourcePanel.getChildren().add(createBankChip(createDeckIcon(), state.getDevelopmentDeck().remainingCount()));
    }

    private VBox createBankChip(Node icon, int amount) {
        VBox chip = new VBox(2);
        chip.getStyleClass().add("bank-resource-chip");
        chip.setAlignment(Pos.CENTER);

        Label amountLabel = new Label(String.valueOf(amount));
        amountLabel.getStyleClass().add("bank-resource-amount");
        chip.getChildren().addAll(icon, amountLabel);
        return chip;
    }

    private Node createDeckIcon() {
        StackPane icon = new StackPane();
        icon.setMinSize(24, 22);
        icon.setPrefSize(24, 22);
        icon.setMaxSize(24, 22);

        Rectangle back = new Rectangle(12, 16);
        back.getStyleClass().add("bank-deck-card-back");
        back.setTranslateX(-3);
        back.setTranslateY(-2);

        Rectangle front = new Rectangle(12, 16);
        front.getStyleClass().add("bank-deck-card-front");
        front.setTranslateX(3);
        front.setTranslateY(2);

        icon.getChildren().addAll(back, front);
        return icon;
    }

    private int visibleTurnNumber(GameState state) {
        if (state.getPhase() == GamePhase.INITIAL_PLACEMENT_FORWARD) {
            return state.getInitialForwardPlacementCount() + 1;
        }
        if (state.getPhase() == GamePhase.INITIAL_PLACEMENT_REVERSE) {
            return state.getPlayers().size() + state.getInitialReversePlacementCount() + 1;
        }
        return Math.max(1, state.getTurnNumber());
    }

    private String turnTypeText(GameState state) {
        if (hasPendingKnightCardSelection()) {
            return "Move Nimon Ungu";
        }
        return switch (state.getPhase()) {
            case INITIAL_PLACEMENT_FORWARD -> "Initial setup - clockwise";
            case INITIAL_PLACEMENT_REVERSE -> "Initial setup - counter-clockwise";
            case ROLL_DICE -> "Roll dice";
            case DISCARD_RESOURCES -> "Discard resources";
            case MOVE_ROBBER -> "Move Nimon Ungu";
            case PLAYER_ACTIONS -> "Trade / build / cards";
            case DETERMINE_START_PLAYER -> "Determine start player";
            case END_TURN -> "End turn";
            case SETUP -> "Setup";
        };
    }

    private void applyHudFont() {
        if (boardHud == null) {
            return;
        }
        var fontUrl = getClass().getResource(HUD_FONT_RESOURCE);
        if (fontUrl == null) {
            return;
        }
        Font loaded = Font.loadFont(fontUrl.toExternalForm(), 13);
        if (loaded != null) {
            boardHud.setStyle("-fx-font-family: \"" + loaded.getFamily().replace("\"", "\\\"") + "\";");
        }
    }

    private void configureActionButtons() {
        refreshPieceActionButtonIcons(getState());
        configureIconButton(settingsButton, ResourceIconFactory.settingsIcon(20), "Settings & Plugins");
        configureIconButton(
                buyNeroifaCardButton,
                createCardStackIcon(),
                "Buy Dr. Neroifa's Card (1 Ore + 1 Wheat + 1 Banana)"
        );
        configureIconButton(tradeButton, createTradeIcon(), "Trade resources");
    }

    private void startBackgroundMusicIfGameActive(GameState state) {
        if (state.getStatus() != GameStatus.GAME_OVER) {
            BackgroundMusicPlayer.getInstance().start();
        }
    }

    private void refreshPieceActionButtonIcons(GameState state) {
        var color = state.getCurrentPlayer().getColor();
        configureIconButton(buildSettlementButton, ResourceIconFactory.iconForPiece("house", color, 25, 25), "Build Pos Pantau");
        configureIconButton(buildPipeButton, ResourceIconFactory.iconForPiece("road", color, 29, 20), "Build Pipa");
        configureIconButton(upgradeLabButton, ResourceIconFactory.iconForPiece("lab", color, 25, 25), "Upgrade ke Laboratorium");
    }

    private void configureIconButton(Button button, Node graphic, String tooltipText) {
        if (button == null) {
            return;
        }
        button.setText("");
        button.setGraphic(graphic);
        button.setTooltip(new Tooltip(tooltipText));
        if (!button.getStyleClass().contains("icon-button")) {
            button.getStyleClass().add("icon-button");
        }
    }

    private Node createCardStackIcon() {
        Pane icon = fixedIconPane(22, 22);
        Rectangle back = new Rectangle(5, 2, 13, 17);
        back.getStyleClass().add("buy-card-icon-back");
        Rectangle front = new Rectangle(2, 5, 13, 17);
        front.getStyleClass().add("buy-card-icon-front");
        icon.getChildren().addAll(back, front);
        return icon;
    }

    private Node createTradeIcon() {
        Pane icon = fixedIconPane(24, 20);
        Line top = new Line(3, 6, 18, 6);
        top.getStyleClass().add("trade-action-icon-line");
        Polygon topArrow = new Polygon(18, 2, 23, 6, 18, 10);
        topArrow.getStyleClass().add("trade-action-icon-arrow");
        Line bottom = new Line(21, 14, 6, 14);
        bottom.getStyleClass().add("trade-action-icon-line");
        Polygon bottomArrow = new Polygon(6, 10, 1, 14, 6, 18);
        bottomArrow.getStyleClass().add("trade-action-icon-arrow");
        icon.getChildren().addAll(top, topArrow, bottom, bottomArrow);
        return icon;
    }

    private Pane fixedIconPane(double width, double height) {
        Pane icon = new Pane();
        icon.setMinSize(width, height);
        icon.setPrefSize(width, height);
        icon.setMaxSize(width, height);
        icon.setMouseTransparent(true);
        return icon;
    }

    private void refreshActionAvailability(GameState state) {
        boolean playerActions = isPlayerActionPhase(state);
        boolean initialSetup = isInitialSetupPhase(state);
        boolean rollDice = state.getStatus() == GameStatus.IN_PROGRESS && state.getPhase() == GamePhase.ROLL_DICE;
        boolean botTurn = isCurrentPlayerBot(state);
        boolean pendingKnightMove = hasPendingKnightCardSelection();
        boolean pendingRoadBuilding = hasPendingRoadBuildingCardSelection();
        boolean pendingCardAction = pendingKnightMove || pendingRoadBuilding;
        setVisibleManaged(rollDiceButton, !initialSetup);
        setVisibleManaged(manualDiceButton, !initialSetup);
        setVisibleManaged(upgradeLabButton, !initialSetup);
        setVisibleManaged(tradeButton, !initialSetup);
        setVisibleManaged(cardsButton, !initialSetup);
        setVisibleManaged(buyNeroifaCardButton, !initialSetup);
        setVisibleManaged(endTurnButton, !initialSetup);
        if (rollDiceButton != null) rollDiceButton.setDisable(pendingCardAction || botTurn || !rollDice);
        if (manualDiceButton != null) manualDiceButton.setDisable(pendingCardAction || botTurn || !rollDice);
        if (buildSettlementButton != null) buildSettlementButton.setDisable(pendingCardAction || botTurn || (!playerActions && !initialSetup));
        if (buildPipeButton != null) buildPipeButton.setDisable(pendingCardAction || botTurn || (!playerActions && !initialSetup));
        if (upgradeLabButton != null) upgradeLabButton.setDisable(pendingCardAction || botTurn || !playerActions);
        if (tradeButton != null) tradeButton.setDisable(pendingCardAction || botTurn || !playerActions);
        if (cardsButton != null) cardsButton.setDisable(pendingCardAction || botTurn || (!playerActions && !rollDice));
        if (buyNeroifaCardButton != null) buyNeroifaCardButton.setDisable(pendingCardAction || botTurn || (!playerActions && !rollDice));
        if (endTurnButton != null) endTurnButton.setDisable(pendingCardAction || botTurn || !playerActions);
    }

    private void setVisibleManaged(Node node, boolean visible) {
        if (node == null) {
            return;
        }
        node.setVisible(visible);
        node.setManaged(visible);
    }

    private boolean isPlayerActionPhase(GameState state) {
        return state.getStatus() == GameStatus.IN_PROGRESS && state.getPhase() == GamePhase.PLAYER_ACTIONS;
    }

    private boolean isTimedTurnPhase(GameState state) {
        return state.getStatus() == GameStatus.IN_PROGRESS
                && (state.getPhase() == GamePhase.PLAYER_ACTIONS || state.getPhase() == GamePhase.MOVE_ROBBER);
    }

    private boolean isInitialSetupPhase(GameState state) {
        return state.getPhase() == GamePhase.INITIAL_PLACEMENT_FORWARD
                || state.getPhase() == GamePhase.INITIAL_PLACEMENT_REVERSE;
    }

    private boolean isRollDicePhase(GameState state) {
        return state.getStatus() == GameStatus.IN_PROGRESS && state.getPhase() == GamePhase.ROLL_DICE;
    }

    private boolean isCurrentPlayerBot(GameState state) {
        return state.getCurrentPlayer() instanceof BotPlayer;
    }

    private boolean shouldAutoPlayCurrentPlayer(GameState state) {
        if (!isCurrentPlayerBot(state) || state.getStatus() == GameStatus.GAME_OVER || victoryDialogAlreadyShown) {
            return false;
        }
        return isInitialSetupPhase(state) || state.getStatus() == GameStatus.IN_PROGRESS;
    }

    private void scheduleBotTurnIfNeeded(GameState state) {
        if (!shouldAutoPlayCurrentPlayer(state) || botTurnRunning || botTurnScheduled) {
            return;
        }
        botTurnScheduled = true;
        Platform.runLater(() -> {
            botTurnScheduled = false;
            processAutomaticBotTurns();
        });
    }

    private void processAutomaticBotTurns() {
        if (botTurnRunning) {
            return;
        }

        botTurnRunning = true;
        boolean completedNormalBotTurn = false;
        int safetyCounter = 0;
        try {
            while (shouldAutoPlayCurrentPlayer(getState()) && safetyCounter < 32) {
                safetyCounter++;
                GameState state = getState();
                BotPlayer bot = (BotPlayer) state.getCurrentPlayer();
                stopTurnTimer(state, true);

                if (isInitialSetupPhase(state)) {
                    if (!autoPlaceInitialSetupForBot(bot)) {
                        break;
                    }
                    refresh(state);
                    continue;
                }

                BotTurnReport report = getBotTurnService().playTurn(getGameEngine(), bot.getStrategy());
                report.messages().forEach(viewModel::appendLog);
                completedNormalBotTurn = completedNormalBotTurn || report.endedTurn();
                refresh(getState());
                evaluateAutomaticGameEvents();
            }

            if (safetyCounter >= 32) {
                viewModel.appendLog("Bot dihentikan sementara karena mencapai batas autoplay.");
            }
        } finally {
            botTurnRunning = false;
        }

        GameState state = getState();
        if (completedNormalBotTurn
                && state.getStatus() == GameStatus.IN_PROGRESS
                && !isCurrentPlayerBot(state)
                && state.getPhase() == GamePhase.ROLL_DICE
                && !victoryDialogAlreadyShown) {
            getGameUiEventBridge().clear();
            AppContext.getCurrent().sceneManager().showTurnTransition();
        }
    }

    private boolean autoPlaceInitialSetupForBot(BotPlayer bot) {
        InitialPlacementChoice choice = findAutomaticInitialPlacement();
        if (choice == null) {
            viewModel.appendLog(bot.getName() + " (bot) gagal menemukan setup awal yang valid.");
            return false;
        }

        GameState state = getState();
        GamePhase previousPhase = state.getPhase();
        getGameEngine().placeInitialSettlementAndPipe(choice.intersectionId(), choice.pathId());
        pendingInitialSettlementId = null;
        if (boardView != null) {
            boardView.clearPendingSettlement();
        }
        buildMode = isInitialSetupPhase(state) ? BuildMode.SETTLEMENT : BuildMode.NONE;
        viewModel.appendLog(bot.getName() + " (bot) menyelesaikan setup Pos dan Pipa.");
        if (state.getPhase() == GamePhase.ROLL_DICE) {
            viewModel.appendLog("Setup awal selesai. Giliran normal dimulai.");
        } else if (previousPhase != state.getPhase()) {
            viewModel.appendLog("Setup masuk putaran berlawanan arah jarum jam.");
        }
        return true;
    }

    private boolean requirePlayerActionPhase(String actionTitle) {
        if (isPlayerActionPhase(getState())) {
            return true;
        }
        AlertUtil.warning(actionTitle, "Aksi ini baru bisa dilakukan setelah roll dice pada giliran pemain.");
        return false;
    }

    private void ensureTurnStarted(GameState state) {
        if (state.getTurnNumber() == 0) {
            state.setTurnNumber(1);
        }
    }

    private Integer promptDieValue(String title, String headerText) {
        TextInputDialog dialog = new TextInputDialog("1");
        dialog.setTitle(title);
        dialog.setHeaderText(headerText);
        return dialog.showAndWait().map(value -> {
            try {
                int die = Integer.parseInt(value.trim());
                if (die < 1 || die > 6) {
                    throw new NumberFormatException();
                }
                return die;
            } catch (NumberFormatException exception) {
                AlertUtil.warning(title, "Nilai dadu harus berupa angka 1 sampai 6.");
                return null;
            }
        }).orElse(null);
    }

    @FXML private void handleRollDice() {
        GameState state = getState();
        if (!isRollDicePhase(state)) {
            AlertUtil.warning("Dadu", "Dadu baru boleh di-roll setelah setup awal selesai dan giliran masuk fase roll.");
            return;
        }
        ensureTurnStarted(state);
        SoundEffectPlayer.getInstance().playDiceRoll();
        DiceRoll diceRoll = getGameEngine().rollDice();
        showDiceRoll(diceRoll);
        viewModel.appendLog(diceRollLogText(state, diceRoll));
        resolveRolledDice(state, diceRoll);
    }

    @FXML private void handleManualDice() {
        if (!isRollDicePhase(getState())) {
            AlertUtil.warning("Dadu", "Dadu manual baru boleh dipakai setelah setup awal selesai dan giliran masuk fase roll.");
            return;
        }

        Integer first = promptDieValue("Dadu manual", "Masukkan nilai dadu pertama (1-6):");
        if (first == null) {
            return;
        }
        Integer second = promptDieValue("Dadu manual", "Masukkan nilai dadu kedua (1-6):");
        if (second == null) {
            return;
        }

        GameState state = getState();
        if (!isRollDicePhase(state)) {
            return;
        }
        ensureTurnStarted(state);
        SoundEffectPlayer.getInstance().playDiceRoll();
        DiceRoll diceRoll = getGameEngine().rollDice(new ManualDiceRoller(first, second));
        showDiceRoll(diceRoll);
        viewModel.appendLog(diceRollLogText(state, diceRoll));
        resolveRolledDice(state, diceRoll);
    }

    private void showDiceRoll(DiceRoll diceRoll) {
        if (diceView != null) {
            diceView.setRoll(diceRoll.getFirstDice().getValue(), diceRoll.getSecondDice().getValue());
        }
    }

    private String diceRollLogText(GameState state, DiceRoll diceRoll) {
        return state.getCurrentPlayer().getName() + " rolls " + diceRoll.getTotal() + "!";
    }

    private void resolveRolledDice(GameState state, DiceRoll diceRoll) {
        if (diceRoll.isSeven()) {
            handleRobberActivated(state);
            return;
        }

        viewModel.appendLog("Resource diproduksi untuk semua petak bernomor " + diceRoll.getTotal() + ".");
        refresh(state);
        evaluateAutomaticGameEvents();
    }

    @FXML private void handleBuildSettlement() {
        if (isInitialSetupPhase(getState())) {
            if (pendingInitialSettlementId != null) {
                AlertUtil.warning("Setup Awal", "Pos Pantau sudah dipilih untuk giliran ini. Pilih Pipa yang menempel untuk menyelesaikan setup.");
                buildMode = BuildMode.PIPE;
                return;
            }
            buildMode = BuildMode.SETTLEMENT;
            return;
        }
        if (!requirePlayerActionPhase("Build Pos")) {
            return;
        }
        buildMode = BuildMode.SETTLEMENT;
    }

    @FXML private void handleBuildPipe() {
        if (isInitialSetupPhase(getState())) {
            if (pendingInitialSettlementId == null) {
                AlertUtil.warning("Setup Awal", "Tempatkan Pos Pantau dulu, baru pilih Pipa yang menempel.");
                buildMode = BuildMode.SETTLEMENT;
                return;
            }
            buildMode = BuildMode.PIPE;
            return;
        }
        if (!requirePlayerActionPhase("Build Pipa")) {
            return;
        }
        buildMode = BuildMode.PIPE;
    }

    @FXML private void handleUpgradeLab() {
        if (!requirePlayerActionPhase("Upgrade Lab")) {
            return;
        }
        buildMode = BuildMode.LABORATORY;
    }

    private void handleIntersectionSelected(IntersectionId intersectionId) {
        if (isInitialSetupPhase(getState())) {
            if (pendingInitialSettlementId != null) {
                AlertUtil.warning("Setup Awal", "Pos Pantau sudah dikunci untuk giliran ini. Pilih Pipa yang menempel.");
                buildMode = BuildMode.PIPE;
                return;
            }
            selectInitialSettlement(intersectionId);
            return;
        }
        if (buildMode == BuildMode.SETTLEMENT) {
            placeSettlement(intersectionId);
        } else if (buildMode == BuildMode.LABORATORY) {
            upgradeLab(intersectionId);
        }
    }

    private void handlePathSelected(PathId pathId) {
        if (hasPendingRoadBuildingCardSelection()) {
            placeRoadBuildingPipe(pathId);
            return;
        }
        if (isInitialSetupPhase(getState())) {
            placeInitialSettlementAndPipe(pathId);
            return;
        }
        if (buildMode == BuildMode.PIPE) {
            placePipe(pathId);
        }
    }

    private void handleHexTileSelected(String hexTileId) {
        if (hasPendingKnightCardSelection()) {
            moveKnightCardRobberToTile(hexTileId);
            return;
        }
        if (getState().getPhase() == GamePhase.MOVE_ROBBER) {
            moveRobberToTile(hexTileId);
        }
    }

    private void selectInitialSettlement(IntersectionId intersectionId) {
        try {
            Intersection intersection = board.getIntersections().get(intersectionId);
            require(intersection != null, "Intersection tidak ditemukan.");
            require(!intersection.hasBuilding(), "Tidak bisa membangun Pos: titik ini sudah ditempati.");
            requireDistanceRule(intersectionId);
            require(findAvailableInitialPipe(intersectionId) != null, "Tidak ada Pipa valid yang menempel pada titik ini.");
            if (!AlertUtil.confirm("Setup Pos", "Tempatkan Pos Pantau di titik ini?")) {
                return;
            }

            pendingInitialSettlementId = intersectionId;
            if (boardView != null) {
                boardView.setPendingSettlement(intersectionId, getState().getCurrentPlayer().getId());
            }
            buildMode = BuildMode.PIPE;
        } catch (RuntimeException ex) {
            AlertUtil.warning("Setup Pos", ex.getMessage());
        }
    }

    private void placeInitialSettlementAndPipe(PathId pathId) {
        try {
            require(pendingInitialSettlementId != null, "Pilih titik Pos Pantau dulu.");
            GameState state = getState();
            GamePhase previousPhase = state.getPhase();
            String playerName = state.getCurrentPlayer().getName();

            getGameEngine().placeInitialSettlementAndPipe(pendingInitialSettlementId, pathId);

            pendingInitialSettlementId = null;
            if (boardView != null) {
                boardView.clearPendingSettlement();
            }
            buildMode = isInitialSetupPhase(state) ? BuildMode.SETTLEMENT : BuildMode.NONE;
            viewModel.appendLog(playerName + " menyelesaikan setup Pos dan Pipa.");
            if (state.getPhase() == GamePhase.ROLL_DICE) {
                viewModel.appendLog("Setup awal selesai. Giliran normal dimulai.");
            } else if (previousPhase != state.getPhase()) {
                viewModel.appendLog("Setup masuk putaran berlawanan arah jarum jam.");
            }
            refresh(state);
        } catch (RuntimeException ex) {
            AlertUtil.warning("Setup Pipa", ex.getMessage());
        }
    }

    private void placeSettlement(IntersectionId intersectionId) {
        try {
            if (!AlertUtil.confirm("Build Pos", "Bangun Pos Pantau di titik ini?")) {
                return;
            }
            GameState state = getState();
            getGameEngine().buildSettlement(intersectionId);
            viewModel.appendLog(state.getCurrentPlayer().getName() + " membangun Pos Pantau.");
            buildMode = BuildMode.NONE;
            refresh(state);
            evaluateAutomaticGameEvents();
        } catch (RuntimeException ex) {
            AlertUtil.warning("Build Pos", ex.getMessage());
        }
    }

    private void placeRoadBuildingPipe(PathId pathId) {
        try {
            PendingRoadBuildingCardSelection selection = getPendingRoadBuildingCardSelection();
            if (selection.contains(pathId)) {
                AlertUtil.warning("Road Building", "Pipa gratis sudah dipilih di path ini.");
                return;
            }
            if (selection.placedCount() == 0) {
                getGameEngine().validateRoadBuildingFirstPipe(selection.cardId(), pathId);
            }

            String pipeOrder = selection.placedCount() == 0 ? "pertama" : "kedua";
            if (!AlertUtil.confirm("Road Building", "Tempatkan Pipa gratis " + pipeOrder + " di sisi ini?")) {
                return;
            }

            GameState state = getState();
            getGameEngine().placeRoadBuildingPipe(selection.cardId(), pathId);
            PendingRoadBuildingCardSelection updatedSelection = selection.withPlacedPath(pathId);
            if (updatedSelection.placedCount() >= ROAD_BUILDING_PIPE_COUNT) {
                getGameEngine().completeRoadBuildingCard(updatedSelection.cardId(), updatedSelection.placedCount());
                clearPendingRoadBuildingCardSelection();
                buildMode = BuildMode.NONE;
                viewModel.appendLog(state.getCurrentPlayer().getName() + " memainkan Road Building dan membangun 2 Pipa gratis.");
                refresh(state);
                evaluateAutomaticGameEvents();
                return;
            }

            registerPendingRoadBuildingCardSelection(updatedSelection);
            buildMode = BuildMode.NONE;
            viewModel.appendLog("Pipa gratis pertama ditempatkan. Pilih Pipa gratis kedua.");
            refresh(state);
        } catch (RuntimeException ex) {
            AlertUtil.warning("Road Building", ex.getMessage());
        }
    }

    private void placePipe(PathId pathId) {
        try {
            if (!AlertUtil.confirm("Build Pipa", "Bangun Pipa Transportasi di sisi ini?")) {
                return;
            }
            GameState state = getState();
            getGameEngine().buildPipe(pathId);
            viewModel.appendLog(state.getCurrentPlayer().getName() + " membangun Pipa Transportasi.");
            buildMode = BuildMode.NONE;
            refresh(state);
            evaluateAutomaticGameEvents();
        } catch (RuntimeException ex) {
            AlertUtil.warning("Build Pipa", ex.getMessage());
        }
    }

    private void upgradeLab(IntersectionId intersectionId) {
        try {
            if (!AlertUtil.confirm("Build Lab", "Upgrade Pos Pantau ini menjadi Laboratorium?")) {
                return;
            }
            GameState state = getState();
            getGameEngine().upgradeSettlementToLaboratory(intersectionId);
            viewModel.appendLog(state.getCurrentPlayer().getName() + " meng-upgrade Pos menjadi Laboratorium.");
            buildMode = BuildMode.NONE;
            refresh(state);
            evaluateAutomaticGameEvents();
        } catch (RuntimeException ex) {
            AlertUtil.warning("Build Lab", ex.getMessage());
        }
    }

    private void requireDistanceRule(IntersectionId intersectionId) {
        for (Path path : board.getPaths().values()) {
            if (!path.touches(intersectionId)) continue;
            IntersectionId other = path.getFirstIntersectionId().equals(intersectionId) ? path.getSecondIntersectionId() : path.getFirstIntersectionId();
            Intersection neighbor = board.getIntersections().get(other);
            require(neighbor == null || !neighbor.hasBuilding(), "Distance rule dilanggar: terlalu dekat dengan Pos/Lab lain.");
        }
    }

    private boolean isConnectedToOwnedNetwork(Path path) {
        return isConnected(path.getFirstIntersectionId()) || isConnected(path.getSecondIntersectionId());
    }

    private boolean isConnected(IntersectionId id) {
        var currentId = getState().getCurrentPlayer().getId();
        Intersection intersection = board.getIntersections().get(id);
        if (intersection != null && intersection.getBuilding().map(b -> b.getOwnerId().equals(currentId)).orElse(false)) return true;
        return board.getPaths().values().stream()
                .filter(p -> p.touches(id))
                .anyMatch(p -> p.getPipe().map(pipe -> pipe.getOwnerId().equals(currentId)).orElse(false));
    }

    private void require(boolean condition, String message) {
        if (!condition) throw new IllegalStateException(message);
    }

    private InitialSetupService getInitialSetupService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(InitialSetupService.class)) {
            return registry.get(InitialSetupService.class);
        }

        InitialSetupService service = new InitialSetupService(getEventBus());
        registry.register(InitialSetupService.class, service);
        return service;
    }

    private BuildService getBuildService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(BuildService.class)) {
            return registry.get(BuildService.class);
        }

        BuildService service = new BuildService(getEventBus());
        registry.register(BuildService.class, service);
        return service;
    }

    private BotTurnService getBotTurnService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(BotTurnService.class)) {
            return registry.get(BotTurnService.class);
        }

        BotTurnService service = new BotTurnService();
        registry.register(BotTurnService.class, service);
        return service;
    }

    private GameEventBus getEventBus() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(GameEventBus.class)) {
            return registry.get(GameEventBus.class);
        }

        GameEventBus eventBus = new GameEventBus();
        registry.register(GameEventBus.class, eventBus);
        return eventBus;
    }

    private TurnManager getTurnManager() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(TurnManager.class)) {
            return registry.get(TurnManager.class);
        }

        TurnManager turnManager = new TurnManager(getEventBus());
        registry.register(TurnManager.class, turnManager);
        return turnManager;
    }

    private VictoryManager getVictoryManager() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(VictoryManager.class)) {
            return registry.get(VictoryManager.class);
        }

        VictoryManager victoryManager = new VictoryManager(getScoreService(), getEventBus());
        registry.register(VictoryManager.class, victoryManager);
        return victoryManager;
    }

    private DiceService getDiceService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(DiceService.class)) {
            return registry.get(DiceService.class);
        }

        DiceService diceService = new DiceService(new RandomDiceRoller(), getEventBus());
        registry.register(DiceService.class, diceService);
        return diceService;
    }

    private ResourceProductionService getResourceProductionService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(ResourceProductionService.class)) {
            return registry.get(ResourceProductionService.class);
        }

        ResourceProductionService service = new ResourceProductionService(getEventBus());
        registry.register(ResourceProductionService.class, service);
        return service;
    }

    private TradeService getTradeService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(TradeService.class)) {
            return registry.get(TradeService.class);
        }

        TradeService tradeService = new TradeService(getEventBus());
        registry.register(TradeService.class, tradeService);
        return tradeService;
    }

    private DiscardService getDiscardService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(DiscardService.class)) {
            return registry.get(DiscardService.class);
        }

        DiscardService service = new DiscardService(getEventBus());
        registry.register(DiscardService.class, service);
        return service;
    }

    private RobberService getRobberService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(RobberService.class)) {
            return registry.get(RobberService.class);
        }

        RobberService service = new RobberService(getEventBus());
        registry.register(RobberService.class, service);
        return service;
    }

    @FXML private void handleTrade() {
        if (!requirePlayerActionPhase("Trade")) {
            return;
        }
        AppContext.getCurrent().sceneManager().dialogManager().showModal("/fxml/dialog-trade.fxml", "Trade Resources");
        refresh(getState());
    }

    @FXML private void handleCards() {
        GameState state = getState();
        if (!isPlayerActionPhase(state)) {
            AlertUtil.warning("Kartu Temuan", "Kartu Temuan baru bisa dilihat setelah roll dice.");
            return;
        }
        AppContext.getCurrent().sceneManager().dialogManager().showModal("/fxml/dialog-cards.fxml", "Kartu Temuan");
        activatePendingKnightCardSelection();
        activatePendingRoadBuildingCardSelection();
        refresh(getState());
        evaluateAutomaticGameEvents();
    }

    private void activatePendingKnightCardSelection() {
        if (!hasPendingKnightCardSelection()) {
            return;
        }
        buildMode = BuildMode.NONE;
        if (boardView != null) {
            boardView.setRobberSelectionMode(true);
        }
        viewModel.appendLog("Pilih tile untuk memindahkan Nimon Ungu.");
    }

    private boolean hasPendingKnightCardSelection() {
        return AppContext.getCurrent().serviceRegistry().contains(PendingKnightCardSelection.class);
    }

    private PendingKnightCardSelection getPendingKnightCardSelection() {
        return AppContext.getCurrent().serviceRegistry().get(PendingKnightCardSelection.class);
    }

    private void clearPendingKnightCardSelection() {
        AppContext.getCurrent().serviceRegistry().unregister(PendingKnightCardSelection.class);
    }

    private void activatePendingRoadBuildingCardSelection() {
        if (!hasPendingRoadBuildingCardSelection()) {
            return;
        }
        buildMode = BuildMode.NONE;
        if (boardView != null) {
            boardView.setRobberSelectionMode(false);
        }
        viewModel.appendLog("Road Building aktif. Pilih Pipa gratis pertama.");
    }

    private boolean hasPendingRoadBuildingCardSelection() {
        return AppContext.getCurrent().serviceRegistry().contains(PendingRoadBuildingCardSelection.class);
    }

    private PendingRoadBuildingCardSelection getPendingRoadBuildingCardSelection() {
        return AppContext.getCurrent().serviceRegistry().get(PendingRoadBuildingCardSelection.class);
    }

    private void registerPendingRoadBuildingCardSelection(PendingRoadBuildingCardSelection selection) {
        AppContext.getCurrent().serviceRegistry().register(PendingRoadBuildingCardSelection.class, selection);
    }

    private void clearPendingRoadBuildingCardSelection() {
        AppContext.getCurrent().serviceRegistry().unregister(PendingRoadBuildingCardSelection.class);
    }

    @FXML private void handleBuyNeroifaCard() {
        GameState state = getState();
        if (!isPlayerActionPhase(state)) {
            AlertUtil.warning("Dr. Neroifa's Card", "Kartu Temuan baru bisa dibeli setelah roll dice.");
            return;
        }
        try {
            OwnedExperimentCard card = getGameEngine().buyExperimentCard();
            viewModel.appendLog(state.getCurrentPlayer().getName() + " membeli Kartu Temuan Dr. Neroifa.");
            AlertUtil.info("Dr. Neroifa's Card", "Kartu berhasil dibeli: " + card.getCard().getCardName());
            refresh(state);
            evaluateAutomaticGameEvents();
        } catch (RuntimeException ex) {
            AlertUtil.warning("Dr. Neroifa's Card", ex.getMessage());
        }
    }

    private CardService getCardService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(CardService.class)) {
            return registry.get(CardService.class);
        }
        CardService service = new CardService(getEventBus());
        registry.register(CardService.class, service);
        return service;
    }

    @FXML private void handleSettings() { AppContext.getCurrent().sceneManager().dialogManager().showModal("/fxml/dialog-settings-plugin.fxml", "Settings & Plugins"); }

    private void handleRobberActivated(GameState state) {
        buildMode = BuildMode.NONE;
        viewModel.appendLog("Dadu 7: Nimon Ungu aktif.");

        if (state.getPhase() == GamePhase.DISCARD_RESOURCES) {
            refresh(state);
            showPendingDiscardDialogs(state);
        }

        if (state.getPhase() == GamePhase.DISCARD_RESOURCES && !getDiscardService().hasPendingDiscards(state)) {
            state.setPhase(GamePhase.MOVE_ROBBER);
        }
        if (boardView != null) {
            boardView.setRobberSelectionMode(true);
        }
        viewModel.appendLog("Pilih tile untuk memindahkan Nimon Ungu sebelum end turn.");
        refresh(state);
    }

    private void evaluateAutomaticGameEvents() {
        GameState state = getState();
        if (victoryDialogAlreadyShown) {
            return;
        }

        if (state.getStatus() == GameStatus.GAME_OVER
                || (state.getStatus() == GameStatus.IN_PROGRESS
                && getGameEngine().checkCurrentPlayerVictory().isPresent())) {
            showVictoryDialog(state);
        }
    }

    private void showVictoryDialog(GameState state) {
        if (victoryDialogAlreadyShown) {
            return;
        }
        victoryDialogAlreadyShown = true;
        stopTurnTimer(state, true);
        BackgroundMusicPlayer.getInstance().stop();
        SoundEffectPlayer.getInstance().playVictoryChime(BackgroundMusicPlayer.getInstance().getVolume());
        AppContext.getCurrent().sceneManager().dialogManager().showModal("/fxml/dialog-victory.fxml", "Declare Victory");
    }

    private ScoreService getScoreService() {
        var registry = AppContext.getCurrent().serviceRegistry();
        if (registry.contains(ScoreService.class)) {
            return registry.get(ScoreService.class);
        }

        ScoreService service = new ScoreService();
        registry.register(ScoreService.class, service);
        return service;
    }

    private boolean hasPlayerExceedingHandLimit(GameState state) {
        return state.getAllPlayers().stream()
                .anyMatch(player -> player.getInventory().getTotalResourceCount() > 7);
    }

    private void showPendingDiscardDialogs(GameState state) {
        DiscardService discardService = getDiscardService();
        while (discardService.hasPendingDiscards(state)) {
            refresh(state);
            AppContext.getCurrent().sceneManager().dialogManager().showModal("/fxml/dialog-discard.fxml", "Discard Resource");
        }
        refresh(state);
    }

    private void moveKnightCardRobberToTile(String hexTileId) {
        GameState state = getState();
        PendingKnightCardSelection selection = getPendingKnightCardSelection();
        try {
            GameEngine engine = getGameEngine();
            List<AbstractPlayer> candidates = engine
                    .getValidRobberStealTargets(hexTileId)
                    .stream()
                    .filter(player -> player.getInventory().getTotalResourceCount() > 0)
                    .toList();
            engine.playKnightCard(selection.cardId(), hexTileId, null);
            clearPendingKnightCardSelection();

            if (boardView != null) {
                boardView.setRobberSelectionMode(false);
            }
            viewModel.appendLog(state.getCurrentPlayer().getName() + " memainkan Nimon Ungu.");
            viewModel.appendLog("Nimon Ungu pindah ke " + hexTileId + ".");
            if (!candidates.isEmpty()) {
                AppContext.getCurrent().sceneManager().dialogManager().showModal("/fxml/dialog-steal-card.fxml", "Steal Resource");
            } else {
                viewModel.appendLog("Tidak ada pencurian resource.");
            }
            refresh(state);
            evaluateAutomaticGameEvents();
        } catch (RuntimeException exception) {
            if (boardView != null) {
                boardView.setRobberSelectionMode(true);
            }
            AlertUtil.warning("Nimon Ungu", exception.getMessage());
        }
    }

    private void moveRobberToTile(String hexTileId) {
        GameState state = getState();
        try {
            GameEngine engine = getGameEngine();
            List<AbstractPlayer> candidates = engine
                    .getValidRobberStealTargets(hexTileId)
                    .stream()
                    .filter(player -> player.getInventory().getTotalResourceCount() > 0)
                    .toList();
            engine.moveRobber(
                    new MoveRobberAction(state.getCurrentPlayer().getId(), hexTileId, null)
            );

            if (boardView != null) {
                boardView.setRobberSelectionMode(false);
            }
            viewModel.appendLog("Nimon Ungu pindah ke " + hexTileId + ".");
            if (!candidates.isEmpty()) {
                AppContext.getCurrent().sceneManager().dialogManager().showModal("/fxml/dialog-steal-card.fxml", "Steal Resource");
            } else {
                viewModel.appendLog("Tidak ada pencurian resource.");
            }
            refresh(state);
            evaluateAutomaticGameEvents();
        } catch (RuntimeException exception) {
            AlertUtil.warning("Nimon Ungu", exception.getMessage());
        }
    }

    @FXML private void handleEndTurn() {
        if (hasPendingKnightCardSelection()) {
            AlertUtil.warning("Nimon Ungu", "Pilih tile tujuan Nimon Ungu dulu.");
            return;
        }
        if (hasPendingRoadBuildingCardSelection()) {
            AlertUtil.warning("Road Building", "Selesaikan penempatan 2 Pipa gratis dulu.");
            return;
        }
        if (!isPlayerActionPhase(getState())) {
            AlertUtil.warning("End Turn", "Giliran hanya bisa diakhiri setelah fase aksi pemain.");
            return;
        }
        advanceTurnAndShowTransition();
    }

    private void advanceTurnAndShowTransition() {
        GameState s = getState();
        if (hasPendingKnightCardSelection()) {
            clearPendingKnightCardSelection();
            if (boardView != null) {
                boardView.setRobberSelectionMode(false);
            }
        }
        stopTurnTimer(s, true);
        getGameEngine().endTurn();
        if (isCurrentPlayerBot(getState())) {
            refresh(getState());
            scheduleBotTurnIfNeeded(getState());
            return;
        }
        getGameUiEventBridge().clear();
        AppContext.getCurrent().sceneManager().showTurnTransition();
    }
}
