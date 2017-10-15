package android.shadowproxy.ListViewEx;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.VelocityTracker;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Scroller;

/**
 * Created by lovefantasy on 2017/6/1.
 */

public class ListViewEx extends ListView {
    private int slidePosition;
    private int downY;
    private int downX;
    private int screenWidth;
    private View itemView;
    private Scroller scroller;
    private final int snap_velocity = 100;
    private boolean isSlide = false;
    private int mTouchSlop;
    private RemoveDirection removeDirection;
    private VelocityTracker velocityTracker;

    public enum RemoveDirection {
        RIGHT, LEFT
    }

    public ListViewEx(Context context) {
        super(context);
        screenWidth = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getWidth();
        scroller = new Scroller((context));
        mTouchSlop = ViewConfiguration.get(context).getScaledTouchSlop();
    }

    public ListViewEx(Context context, AttributeSet attrs) {
        super(context, attrs);
        screenWidth = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getWidth();
        scroller = new Scroller((context));
        mTouchSlop = ViewConfiguration.get(context).getScaledTouchSlop();
    }

    public ListViewEx(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        screenWidth = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getWidth();
        scroller = new Scroller((context));
        mTouchSlop = ViewConfiguration.get(context).getScaledTouchSlop();


    }


    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        switch (ev.getAction()) {
            case MotionEvent.ACTION_DOWN:
                addVelocityTracker(ev);

                if (!scroller.isFinished()) {
                    return super.dispatchTouchEvent(ev);
                }
                downX = (int) ev.getX();
                downY = (int) ev.getY();
                slidePosition = pointToPosition(downX, downY);

                if (slidePosition == AdapterView.INVALID_POSITION) {
                    return super.dispatchTouchEvent(ev);
                }
                itemView = getChildAt(slidePosition - getFirstVisiblePosition());

                break;

            case MotionEvent.ACTION_MOVE:
                if (Math.abs(getscrollVelocity()) > snap_velocity ||
                        (Math.abs(ev.getX() - downX) > mTouchSlop && Math.abs(ev.getY() - downY) < mTouchSlop)) {
                    isSlide = true;
                }
                break;

            case MotionEvent.ACTION_UP:
                recycleVelocityTracker();

                break;
        }
        return super.dispatchTouchEvent(ev);
    }

    private void scrollRight() {
        final int delta = (itemView.getScrollX()-((ViewGroup) itemView).getChildAt(1).getWidth());
        //LogTools.e("scrollright",String.valueOf(delta));

        scroller.startScroll(itemView.getScrollX(), 0, -delta, 0, 200);
        postInvalidate();
    }

    private void scrollLeft() {
        final int delta = (itemView.getScrollX() - ((ViewGroup) itemView).getChildAt(1).getWidth());
       // LogTools.e("scrollleft", String.valueOf(1));

        scroller.startScroll(itemView.getScrollX(), 0, -delta, 0, 200);
        postInvalidate();
    }

    private void scrollByDistanceX() {
        if (itemView.getScrollX() >= ((ViewGroup) itemView).getChildAt(1).getWidth()) {
            scrollLeft();

        }  if (itemView.getScrollX() < ((ViewGroup) itemView).getChildAt(1).getWidth()) {
            // scrollLeft();
            itemView.scrollTo(0, 0);

        }/* else if (itemView.getScrollX() <0) {
            //scrollRight();
            itemView.scrollTo(0, 0);
        } else {
           // itemView.scrollTo(0, 0);
        }*/
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        if (isSlide && slidePosition != AdapterView.INVALID_POSITION) {
            requestDisallowInterceptTouchEvent(true);
            addVelocityTracker(ev);
            final int action = ev.getAction();
            int x = (int) ev.getX();
            switch (action) {
                case MotionEvent.ACTION_DOWN:
                    break;
                case MotionEvent.ACTION_MOVE:
                    MotionEvent cancelEvent = MotionEvent.obtain(ev);
                    cancelEvent.setAction(MotionEvent.ACTION_CANCEL | (ev.getActionIndex() <<
                            MotionEvent.ACTION_POINTER_INDEX_SHIFT));
                    onTouchEvent(cancelEvent);
                    int deltaX = downX - x;
                    downX = x;
                    itemView.scrollBy(deltaX, 0);

                    return true;
                case MotionEvent.ACTION_UP:
                    int velcityX = getscrollVelocity();
                    if (velcityX > snap_velocity) {
                        scrollRight();
                    } else if (velcityX < -snap_velocity) {
                        scrollLeft();
                    } else {
                        scrollByDistanceX();
                    }

                    recycleVelocityTracker();
                    isSlide = false;
                    break;
            }


        }


        return super.onTouchEvent(ev);
    }

    @Override
    public void computeScroll() {
        super.computeScroll();
        if (scroller.computeScrollOffset()) {
            itemView.scrollTo(scroller.getCurrX(), scroller.getCurrY());
            postInvalidate();
            /*if (scroller.isFinished()) {
                itemView.scrollTo(0, 0);
            }*/
        }


    }

    private void addVelocityTracker(MotionEvent ev) {
        if (velocityTracker == null) {
            velocityTracker = VelocityTracker.obtain();
        }
        velocityTracker.addMovement(ev);
    }

    private void recycleVelocityTracker(){
        if (velocityTracker != null) {
            velocityTracker.recycle();
            velocityTracker = null;
        }
    }
    private int getscrollVelocity(){
        velocityTracker.computeCurrentVelocity(1000);
        int velocity = (int) velocityTracker.getXVelocity();
        return velocity;
    }


}
