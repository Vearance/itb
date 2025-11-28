package algeo.modules;

public class SPLResult {
    public final Matrix solution;
    public final String steps;

    public SPLResult(Matrix solution, String steps) {
        this.solution = solution;
        this.steps = steps;
    }
}
