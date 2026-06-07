package com.bananarepublic.timer;

import java.util.Objects;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.function.IntConsumer;

public final class CountdownTimer {
    private final int durationSeconds;
    private final TimerTaskRunner taskRunner;
    private final IntConsumer tickHandler;
    private final Runnable expiredHandler;
    private final Object lock = new Object();
    private ScheduledFuture<?> scheduledTask;
    private int remainingSeconds;
    private TimerState state = TimerState.READY;

    public CountdownTimer(int durationSeconds, TimerTaskRunner taskRunner, IntConsumer tickHandler, Runnable expiredHandler) {
        if (durationSeconds <= 0) {
            throw new IllegalArgumentException("Timer duration must be positive");
        }
        this.durationSeconds = durationSeconds;
        this.taskRunner = Objects.requireNonNull(taskRunner, "taskRunner");
        this.tickHandler = Objects.requireNonNull(tickHandler, "tickHandler");
        this.expiredHandler = Objects.requireNonNull(expiredHandler, "expiredHandler");
        this.remainingSeconds = durationSeconds;
    }

    // calling runnable
    public void start() {
        synchronized (lock) {
            cancelScheduledTask();
            remainingSeconds = durationSeconds;
            state = TimerState.RUNNING;
            scheduledTask = taskRunner.scheduleAtFixedRate(this::tick, 1, 1, TimeUnit.SECONDS);
        }
    }

    public void stop() {
        synchronized (lock) {
            if (state == TimerState.RUNNING) {
                state = TimerState.STOPPED;
            }
            cancelScheduledTask();
        }
    }

    public int getRemainingSeconds() {
        synchronized (lock) {
            return remainingSeconds;
        }
    }

    public TimerState getState() {
        synchronized (lock) {
            return state;
        }
    }

    private void tick() {
        int secondsLeft;
        boolean expired;

        synchronized (lock) {
            if (state != TimerState.RUNNING) {
                return;
            }

            remainingSeconds--;
            if (remainingSeconds <= 0) {
                remainingSeconds = 0;
                state = TimerState.EXPIRED;
                cancelScheduledTask();
                expired = true;
            } else {
                expired = false;
            }
            secondsLeft = remainingSeconds;
        }

        tickHandler.accept(secondsLeft);
        if (expired) {
            expiredHandler.run();
        }
    }

    private void cancelScheduledTask() {
        if (scheduledTask != null) {
            scheduledTask.cancel(false);
            scheduledTask = null;
        }
    }
}
