package com.yunjin.application.unit;

import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import com.yunjin.application.R;

public class JoystickView extends View {
    private Paint circlePaint, handlePaint;
    private float centerX, centerY, handleRadius;
    private OnJoystickMoveListener listener;

    private float currentHandleX, currentHandleY;
    private ValueAnimator resetAnimator;


    public interface OnJoystickMoveListener {
        void onValueChanged(int type, float xPercent, float yPercent);
    }

    public JoystickView(Context context) {
        super(context);
        init();
    }
    public JoystickView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }
    public JoystickView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    public void setListener(OnJoystickMoveListener listener) {
        this.listener = listener;
    }

    private void init() {
        circlePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        circlePaint.setColor(Color.GRAY);
        circlePaint.setStyle(Paint.Style.FILL_AND_STROKE);

        handlePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        handlePaint.setColor(Color.RED);
        handlePaint.setStyle(Paint.Style.FILL);
        // 在 init() 中添加
        circlePaint.setColor(Color.parseColor("#334CAF50"));  // 修改外圈颜色
        handlePaint.setColor(Color.parseColor("#FF4081"));  // 修改手柄颜色
        resetAnimator = ValueAnimator.ofFloat(1f, 0f);
        resetAnimator.setDuration(150);
        resetAnimator.addUpdateListener(animation -> {
            float fraction = (float) animation.getAnimatedValue();
            currentHandleX = centerX + (currentHandleX - centerX) * fraction;
            currentHandleY = centerY + (currentHandleY - centerY) * fraction;
            invalidate();
        });
    }
    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        centerX = w / 2f;
        centerY = h / 2f;
        handleRadius = Math.min(w, h) / 5f;
        // 初始化手柄位置到中心
        currentHandleX = centerX;
        currentHandleY = centerY;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        // 绘制外圈（保持原样）
        canvas.drawCircle(centerX, centerY, handleRadius * 2, circlePaint);
        // 修改这里：使用动态位置绘制手柄
        canvas.drawCircle(currentHandleX, currentHandleY, handleRadius, handlePaint);
    }


    @Override
    public boolean onTouchEvent(MotionEvent event) {
        float x = event.getX();
        float y = event.getY();
        float dx = x - centerX;
        float dy = y - centerY;
        float distance = (float) Math.sqrt(dx*dx + dy*dy);

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_MOVE:
                if (distance > handleRadius*2) {
                    dx *= (handleRadius*2) / distance;
                    dy *= (handleRadius*2) / distance;
                }
                // 更新手柄实际位置
                currentHandleX = centerX + dx;
                currentHandleY = centerY + dy;
                break;

            case MotionEvent.ACTION_UP:
                // 启动回弹动画
                if (resetAnimator.isRunning()) resetAnimator.cancel();
                resetAnimator.start();
                break;
        }

        // 计算百分比并回调
        float xPercent = (currentHandleX - centerX) / (handleRadius*2);
        float yPercent = (currentHandleY - centerY) / (handleRadius*2);

        if(listener != null) {
            int type = (getId() == R.id.left_joystick) ? 0 : 1;
            listener.onValueChanged(type, xPercent, yPercent);
        }

        invalidate();
        return true;
    }

}