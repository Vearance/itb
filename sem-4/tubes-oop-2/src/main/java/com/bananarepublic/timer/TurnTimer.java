package com.bananarepublic.timer;

import com.bananarepublic.core.player.PlayerId;
import com.bananarepublic.event.GameEventBus;
import com.bananarepublic.event.TimerExpiredEvent;
import com.bananarepublic.event.TimerStartedEvent;
import com.bananarepublic.event.TimerTickEvent;

import java.util.Objects;
import java.util.function.IntConsumer;

public final class TurnTimer {
    public static final int ROUND_DURATION = 90;

    private final int durationSeconds;
    private final GameEventBus eventBus;
    private final TimerTaskRunner taskRunner;
    private CountdownTimer countdownTimer;

    public TurnTimer(GameEventBus eventBus) {
        this(ROUND_DURATION, eventBus, new TimerTaskRunner());
    }

    public TurnTimer(int durationSeconds, GameEventBus eventBus) {
        this(durationSeconds, eventBus, new TimerTaskRunner());
    }

    public TurnTimer(int durationSeconds, GameEventBus eventBus, TimerTaskRunner taskRunner) {
        if (durationSeconds <= 0) {
            throw new IllegalArgumentException("Timer duration must be positive");
        }
        this.durationSeconds = durationSeconds;
        this.eventBus = Objects.requireNonNull(eventBus, "eventBus");
        this.taskRunner = Objects.requireNonNull(taskRunner, "taskRunner");
    }

    public synchronized void start(PlayerId playerId, int turnNumber, Runnable onExpired) {
        start(playerId, turnNumber, durationSeconds, remainingSeconds -> {}, onExpired);
    }

    public synchronized void start(PlayerId playerId, int turnNumber, int initialRemainingSeconds, Runnable onExpired) {
        start(playerId, turnNumber, initialRemainingSeconds, remainingSeconds -> {}, onExpired);
    }

    public synchronized void start(
            PlayerId playerId,
            int turnNumber,
            int initialRemainingSeconds,
            IntConsumer tickConsumer,
            Runnable onExpired
    ) {
        Objects.requireNonNull(playerId, "playerId");
        Objects.requireNonNull(tickConsumer, "tickConsumer");
        Objects.requireNonNull(onExpired, "onExpired");
        if (initialRemainingSeconds <= 0) {
            throw new IllegalArgumentException("Timer duration must be positive");
        }
        stop();

        countdownTimer = new CountdownTimer(
                initialRemainingSeconds,
                taskRunner,
                remainingSeconds -> {
                    eventBus.publish(new TimerTickEvent(playerId, turnNumber, remainingSeconds));
                    tickConsumer.accept(remainingSeconds);
                },
                () -> {
                    eventBus.publish(new TimerExpiredEvent(playerId, turnNumber));
                    onExpired.run();
                }
        );
        eventBus.publish(new TimerStartedEvent(playerId, turnNumber, initialRemainingSeconds));
        countdownTimer.start();
    }

    public synchronized void stop() {
        if (countdownTimer != null) {
            countdownTimer.stop();
        }
    }

    public synchronized int getRemainingSeconds() {
        if (countdownTimer == null) {
            return 0;
        }
        return countdownTimer.getRemainingSeconds();
    }

    public synchronized TimerState getState() {
        if (countdownTimer == null) {
            return TimerState.READY;
        }
        return countdownTimer.getState();
    }

    public synchronized void shutdown() {
        stop();
        taskRunner.shutdown();
    }
}
