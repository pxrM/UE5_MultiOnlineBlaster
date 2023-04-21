ue5的 AimOffset原理
	在UE5中，AimOffset是一种常用的动画融合方式，用于实现角色在瞄准时身体的姿势调整。具体来说，AimOffset会根据角色当前的朝向、瞄准方向等参数，
	计算出一个偏移值，然后将该偏移值应用到角色的骨骼动画中，从而实现角色在瞄准时身体的姿势调整。
	AimOffset的实现原理主要分为以下几个步骤：

		1.创建AimOffset动画：首先需要创建一个AimOffset动画，该动画主要包括各种角色姿势的变化，这些变化是根据瞄准方向和角色朝向等参数计算出来的。

		2.设置AimOffset属性：接下来需要在角色的动画蓝图中，将AimOffset动画和相关的参数进行绑定，从而实现AimOffset的自动计算和应用。

		3.计算AimOffset值：当角色进行瞄准操作时，会通过代码计算出瞄准方向和角色朝向等参数，并将其传递给AimOffset系统。

		4.应用AimOffset值：AimOffset系统会根据传入的参数，自动计算出一个偏移值，并将该偏移值应用到角色的骨骼动画中，从而实现角色在瞄准时身体的姿势调整。

	总体来说，AimOffset是一种便捷、高效的动画融合方式，在游戏开发中得到了广泛的应用。通过使用AimOffset，可以轻松实现角色在瞄准时身体的姿势调整，从而使游戏更加逼真和流畅。
	
	
将多个姿势混合成一个复杂的姿势
	将多个姿势混合成一个复杂的姿势是动画制作中的常见需求。一种常见的做法是使用蒙太奇（blending）技术，将多个基础动画混合在一起，生成一个新的复杂动画。
	在 Unity 引擎中，可以使用 Mecanim 动画系统来实现动画蒙太奇。具体地，可以将多个动作分别定义为对应的动画状态（Animation State），每个状态对应一个动画片段（Animation Clip）。
	然后，使用 Mecanim 的机制来控制状态之间的转换，以实现多个动作的混合。
	
	在 Unreal Engine 引擎中，可以使用 Animation Blueprint 来实现动画蒙太奇。具体地，可以将多个动作分别定义为对应的动画 Montage，
	然后在 Animation Blueprint 中使用 Blend node 或 Layered Blend node 来混合这些 Montage。Blend node 会简单地线性混合两个 Montage，
	而 Layered Blend node 可以更灵活地控制混合的方式，例如可以设置每个 Montage 的权重、融合时间等参数。
	
	
ue5Fabrik	https://docs.unrealengine.com/5.0/en-US/fabrik-animation-blueprint-in-unreal-engine/
	UE5 中的 Fabrik 是 Inverse Kinematics（逆向运动学）系统，它可以帮助制作者实现骨骼动画。Fabrik 的全称是 Forward and Backward Reaching Inverse Kinematics（正向和反向逆运动学），是一种比较流行的动画技术。
	Fabrik 可以处理以下情况：
		1.当手握某个物体时，手臂的位置和旋转会自然地调整，使得角色看起来更加真实。
		2.在进行足部 IK 时，Fabrik 可以确保脚跟和脚尖保持在地面上，并且膝盖可以弯曲。
		3.在进行头部 IK 时，会自动调整身体和头部的姿势，使得角色看起来更自然。
		UE5 的 Fabrik 系统非常灵活，允许开发者通过简单的节点操作来创建交互式的 IK 动画效果。同时，它还支持多种不同类型的约束条件，如长度限制、方向控制等，开发者可以根据需要进行设置。