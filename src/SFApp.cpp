#include "SFApp.h"

SFApp::SFApp(std::shared_ptr<SFWindow> window) : fire(0), is_running(true), sf_window(window) {
  int canvas_w, canvas_h;
  SDL_GetRendererOutputSize(sf_window->getRenderer(), &canvas_w, &canvas_h);

  app_box = make_shared<SFBoundingBox>(Vector2(canvas_w, canvas_h), canvas_w, canvas_h);
  player  = make_shared<SFAsset>(SFASSET_PLAYER, sf_window);
  auto player_pos = Point2(canvas_w/2.0f, 88.0f);
  player->SetPosition(player_pos);
  player->SetHealth(500);

  const int number_of_aliens = 3;
  for(int i=0; i<number_of_aliens; i++) {
    // place an alien at width/number_of_aliens * i
    auto alien = make_shared<SFAsset>(SFASSET_ALIEN, sf_window);
    auto pos   = Point2((canvas_w/number_of_aliens) * i + 48.0f, 300.0f);
    alien->SetPosition(pos);
    alien->SetHealth(100);
    aliens.push_back(alien);
  }

  auto coin = make_shared<SFAsset>(SFASSET_COIN, sf_window);
  auto pos  = Point2((canvas_w/4), 100);
  coin->SetPosition(pos);
  coins.push_back(coin);

  cout << "You have" << player->GetHealth() << " HP." << endl;
}

SFApp::~SFApp() {
}

/**
 * Handle all events that come from SDL.
 * These are timer or keyboard events.
 */
void SFApp::OnEvent(SFEvent& event) {
  SFEVENT the_event = event.GetCode();
  switch (the_event) {
  case SFEVENT_QUIT:
    is_running = false;
    break;
  case SFEVENT_UPDATE:
    OnUpdateWorld();
    OnRender();
    break;
  case SFEVENT_PLAYER_LEFT:
    player->GoWest();
    break;
  case SFEVENT_PLAYER_RIGHT:
    player->GoEast();
    break;
  case SFEVENT_PLAYER_UP:
    player->GoNorth();
    break;
  case SFEVENT_PLAYER_DOWN:
    player->GoSouth();
    break;
  case SFEVENT_FIRE:
    fire ++;
    FireProjectile();
    break;
  }
}

int SFApp::OnExecute() {
  // Execute the app
  SDL_Event event;
  while (SDL_WaitEvent(&event) && is_running) {
    // wrap an SDL_Event with our SFEvent
    SFEvent sfevent((const SDL_Event) event);
    // handle our SFEvent
    OnEvent(sfevent);
  }
}
void SFApp::EndGame(){
 if(player->GetHealth() <= 0){
    cout << "GAME OVER!" << endl;
    EndGame();
    is_running = false;
  }
}

void SFApp::OnUpdateWorld() {
  int w, h;
  SDL_GetRendererOutputSize(sf_window->getRenderer(), &w, &h);

  // Update projectile positions

  for(auto p: projectiles) {
    p->GoNorth();
    
    auto p_pos = p->GetPosition();

    if(p_pos.getY() > h) {
      p->HandleCollision();
    }
  }


  for(auto c: coins) {
    c->GoNorth();

    //check collision with coins
    if(player->CollidesWith(c)) {
      c->HandleCollision();
      cout << "Coin Collected!" << endl;
    }
  }


  // Update enemy positions
  for(auto a : aliens) {
    a->GoSouth();
    if(player->CollidesWith(a)) {
      player->SetHealth(player->GetHealth() - 50);
      if(a->HandleCollision() == 1){
	enemiesKilled++;
      }
      cout << "You were hit! Remaining HP" << player->GetHealth() << endl;
    }
  }


  // Detect collisions
  for(auto a : aliens) {
    for(auto p : projectiles) {
      if(p->CollidesWith(a)) {
        p->HandleCollision();
	enemiesKilled++;
      }
    }
  }

  for(auto p : projectiles) {
    for(auto c : coins) {
      if(p->CollidesWith(c)) {
	cout << "Coin Collected!" << endl;
	p->HandleCollision();
	c->HandleCollision();
      }
    }
  }

  // remove dead aliens (the long way)
  list<shared_ptr<SFAsset>> tmp;
  for(auto a : aliens) {
    if(a->IsAlive()) {
      tmp.push_back(a);
    }
  }
  aliens.clear();
  aliens = list<shared_ptr<SFAsset>>(tmp);

  //remove coin when hit(long way)
  for(auto c : coins) {
    for(auto p : projectiles){
      if(p->CollidesWith(c)) {
	c->HandleCollision();
      }
    }
  }
  
   list<shared_ptr<SFAsset>> coin_remove;
    for(auto c : coins) {
      if(c->IsAlive()) {
	coin_remove.push_back(c);
      }
    }
  coins.clear();
  coins = list<shared_ptr<SFAsset>>(coin_remove);


}

void SFApp::OnRender() {
  int w, h;
  SDL_RenderClear(sf_window->getRenderer());

  // draw the player
  player->OnRender();

  for(auto p: projectiles) {
    if(p->IsAlive()) {p->OnRender();}
  }

  for(auto a: aliens) {
    if(a->IsAlive()) {a->OnRender();}
  }

  for(auto c: coins) {
    if(c->IsAlive()){c->OnRender();}
  }



  // Switch the off-screen buffer to be on-screen
  SDL_RenderPresent(sf_window->getRenderer());
}

void SFApp::FireProjectile() {
  auto pb = make_shared<SFAsset>(SFASSET_PROJECTILE, sf_window);
  auto v  = player->GetPosition();
  pb->SetPosition(v);
  projectiles.push_back(pb);
}
