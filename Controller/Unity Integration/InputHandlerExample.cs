using System.Linq;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.LowLevel;

public class InputHandlerExample : MonoBehaviour
{
    [Header("Settings")]
    public bool useMicrotwiController = true;

    private void Update()
    {
        if (MicrotwiMobileController.Instance != null && MicrotwiMobileController.Instance.microtwiGamepad != null && useMicrotwiController)
        {
            string[] strings = MicrotwiMobileController.Instance.output.Split(' ');

            GamepadState gamepadState = new GamepadState();

            for (int i = 0; i < strings.Count(); i++)
            {
                string[] test = strings[i].Split('_');
                switch (test[0])
                {
                    case "MOVE":
                        float.TryParse(test[1], out float moveBtn);
                        float.TryParse(test[2], out float moveX);
                        float.TryParse(test[3], out float moveY);

                        if (moveX >= -1 && moveX <= 1)
                        {
                            moveX = 0;
                        }
                        else
                        {
                            moveX /= 10;
                        }

                        if (moveY >= -1 && moveY <= 1)
                        {
                            moveY = 0;
                        }
                        else
                        {
                            moveY /= 10;
                        }

                        gamepadState.leftStick = new Vector2(moveX, moveY);

                        if (moveBtn > 0.3f)
                        {
                            // Ex - Sprint Function
                        }

                        break;
                    case "CAM":
                        float.TryParse(test[2], out float camX);
                        float.TryParse(test[3], out float camY);

                        if (camX >= -1 && camX <= 1)
                        {
                            camX = 0;
                        }
                        else
                        {
                            camX /= 10;
                        }

                        if (camY >= -1 && camY <= 1)
                        {
                            camY = 0;
                        }
                        else
                        {
                            camY /= 10;
                        }

                        gamepadState.rightStick = new Vector2(camX, camY);

                        break;
                    case "BTN":
                        test[1] = test[1].Trim();
                        switch (test[1])
                        {
                            case "Y":
                                gamepadState.buttons = 1 << 4;
                                break;
                            case "B":
                                gamepadState.buttons = 1 << 5;
                                break;
                            case "P":
                                gamepadState.buttons = 1 << 12;
                                break;
                            case "S":
                                gamepadState.buttons = 1 << 13;
                                break;
                            case "X":
                                gamepadState.buttons = 1 << 7;
                                break;
                            case "A":
                                gamepadState.buttons = 1 << 6;
                                break;
                            default:
                                gamepadState.buttons = 0;
                                break;
                        }
                        break;
                }
            }

            InputSystem.QueueStateEvent(MicrotwiMobileController.Instance.microtwiGamepad, gamepadState);
        }
    }
}
