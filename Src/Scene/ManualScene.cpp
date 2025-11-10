#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Object/UIInput.h"
#include "../Manager/SceneManager.h"
#include "ManualScene.h"

ManualScene::ManualScene(void)
{
}

ManualScene::~ManualScene(void)
{
}

void ManualScene::Init(void)
{
	page_ = 0;
}

void ManualScene::Update(void)
{
    auto mi = UIInput::GetManualInput();
    auto& scene = SceneManager::GetInstance();

    // ページ送り（任意）
    if (mi.prevPage && page_ > 0) page_--;
    if (mi.nextPage) page_++;

    // 遷移
    if (mi.backToTitle) {
        scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
        return;
    }
    if (mi.goPlayerSelect) {
        scene.ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);
        return;
    }
}

void ManualScene::Draw(void)
{
    DrawFormatString(80, 60, GetColor(255, 255, 255), "操作説明 (page=%d)", page_);
    DrawFormatString(80, 100, GetColor(200, 200, 200), "A(Enter): 人数選択へ");
    DrawFormatString(80, 120, GetColor(200, 200, 200), "B(Space): タイトルへ");
    //DrawFormatString(80, 140, GetColor(200, 200, 200), "Y/→: 次ページ, X/←: 前ページ");
}

void ManualScene::Release(void)
{
}
