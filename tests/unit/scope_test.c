#include <test.h>
#include <scope.h>
#include <env_context.h>
#include <rlist.h>

static void test_name_join(void)
{
    {
        char buf[CF_MAXVARSIZE] = { 0 };
        JoinScopeName(NULL, "sys", buf);
        assert_string_equal("sys", buf);
    }

    {
        char buf[CF_MAXVARSIZE] = { 0 };
        JoinScopeName("ns", "b", buf);
        assert_string_equal("ns:b", buf);
    }
}

static void test_name_split(void)
{
    {
        char ns[CF_MAXVARSIZE] = { 0 };
        char b[CF_MAXVARSIZE] = { 0 };
        SplitScopeName("sys", ns, b);
        assert_string_equal("", ns);
        assert_string_equal("sys", b);
    }

    {
        char ns[CF_MAXVARSIZE] = { 0 };
        char b[CF_MAXVARSIZE] = { 0 };
        SplitScopeName("ns:b", ns, b);
        assert_string_equal("ns", ns);
        assert_string_equal("b", b);
    }
}

static void test_push_pop_this(void)
{
    VarRef *lval = VarRefParse("this.lval");
    Rval rval;

    EvalContext *ctx = EvalContextNew();

    Policy *p = PolicyNew();
    Bundle *bp = PolicyAppendBundle(p, "default", "bundle", "agent", NULL, NULL);
    PromiseType *pt = BundleAppendPromiseType(bp, "reports");
    Promise *pp = PromiseTypeAppendPromise(pt, "hello", (Rval) { NULL, RVAL_TYPE_NOPROMISEE }, NULL);

    EvalContextStackPushBundleFrame(ctx, bp, NULL, false);
    EvalContextStackPushPromiseFrame(ctx, pp);
    EvalContextStackPushPromiseIterationFrame(ctx, pp);

    ScopeNewSpecial(ctx, SPECIAL_SCOPE_THIS, "lval", "rval1", DATA_TYPE_STRING);
    assert_true(EvalContextVariableGet(ctx, lval, &rval, NULL));
    assert_string_equal("rval1", RvalScalarValue(rval));
    {
        ScopePushThis();

        assert_false(EvalContextVariableGet(ctx, lval, &rval, NULL));

        ScopeNewSpecial(ctx, SPECIAL_SCOPE_THIS, "lval", "rval2", DATA_TYPE_STRING);
        assert_true(EvalContextVariableGet(ctx, lval, &rval, NULL));
        assert_string_equal("rval2", RvalScalarValue(rval));
        {
            ScopePushThis();

            assert_false(EvalContextVariableGet(ctx, lval, &rval, NULL));

            ScopeNewSpecial(ctx, SPECIAL_SCOPE_THIS, "lval", "rval3", DATA_TYPE_STRING);
            assert_true(EvalContextVariableGet(ctx, lval, &rval, NULL));
            assert_string_equal("rval3", RvalScalarValue(rval));
            {

            }

            ScopePopThis();
        }

        ScopePopThis();
    }
    assert_true(EvalContextVariableGet(ctx, lval, &rval, NULL));
    assert_string_equal("rval1", RvalScalarValue(rval));

    EvalContextStackPopFrame(ctx);
    EvalContextStackPopFrame(ctx);
    EvalContextStackPopFrame(ctx);

    PolicyDestroy(p);
    EvalContextDestroy(ctx);
}

int main()
{
    const UnitTest tests[] =
{
        unit_test(test_name_split),
        unit_test(test_name_join),
        unit_test(test_push_pop_this),
    };

    PRINT_TEST_BANNER();
    return run_tests(tests);
}
